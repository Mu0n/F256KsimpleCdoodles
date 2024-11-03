#define F256LIB_IMPLEMENTATION

#define MIDI_CTRL 0xDDA0
#define MIDI_OUT 0xDDA1
#define MIDI_RX_1 0xDDA2
#define MIDI_RX_2 0xDDA3

#define TIMER_FRAMES 0
#define TIMER_SECONDS 1

 //TIMER_TEXT for the 1-frame long text refresh timer for data display
 //TIMER_NOTE is for a midi note timer
#define TIMER_RX_COOKIE 0
#define TIMER_GUI_COOKIE 1
#define TIMER_RX_DELAY 1
#define TIMER_GUI_DELAY 1

#define SYS_CTRL_TEXT_PG 2
#define SYS_CTRL_1 0x0001

#include "f256lib.h"
struct timer_t midiRxTimer, GUITimer; //timer_t structure for setting timer through the kernel

uint16_t note = 0x36, oldnote; /*note is the current midi hex note code to send. oldnote keeps the previous one so it can be Note_off'ed away after the timer expires, or a new note is called*/
uint16_t prgInst = 0; /* program change value, the MIDI instrument number */

bool setTimer(const struct timer_t *timer)
{
    *(uint8_t*)0xf3 = timer->units;
    *(uint8_t*)0xf4 = timer->absolute;
    *(uint8_t*)0xf5 = timer->cookie;
    kernelCall(Clock.SetTimer);
	return !kernelError;
}

uint8_t getTimerAbsolute(uint8_t units)
{
    *(uint8_t*)0xf3 = units | 0x80;
    return kernelCall(Clock.SetTimer);
}

void setInstruments()
{
	POKE(MIDI_OUT,123);
	POKE(MIDI_OUT,0);
	POKE(MIDI_OUT,0xFF);
}
void setup()
{
	setInstruments();
	GUITimer.units = TIMER_SECONDS;
	GUITimer.absolute = TIMER_GUI_DELAY;
	GUITimer.cookie = TIMER_GUI_COOKIE;
	
	midiRxTimer.units = TIMER_FRAMES;
	midiRxTimer.absolute = TIMER_RX_DELAY;
	midiRxTimer.cookie = TIMER_RX_COOKIE;
	setTimer(&midiRxTimer);
	setTimer(&GUITimer);
}
void fetchNextEventMIDI()
{
	uint16_t count=0;
	
	count = count | PEEK(MIDI_RX_1);
	count = count | (uint16_t)(PEEK(MIDI_RX_2)>>8);
	
	printf("count %d ",count);
	if(count > 0) POKE(MIDI_OUT, PEEK(MIDI_OUT));
}
void midiNoteOn(uint8_t wantNote, uint8_t chan)
{
	//Send a Note_On midi command on channel 0		
	POKE(MIDI_OUT, 0x90 | chan);
	POKE(MIDI_OUT, wantNote);
	POKE(MIDI_OUT, 0x7F);
}


void irqHandler()
{
	uint8_t var,keepCtrl;
	
	keepCtrl = PEEK(1);
	POKE(SYS_CTRL_1,0);
	var = CHECK_BIT(INT_PEND_0,INT00_VKY_SOF);
	if(var==1)
	{
		POKE(INT_PEND_0,var);
		POKE(SYS_CTRL_1,SYS_CTRL_TEXT_PG);
		POKE(0xC000,PEEK(0xC000)+1);
	}
	POKE(1,keepCtrl);
}


int main(int argc, char *argv[]) {
	//setup();

	while(true) 
        {
	asm("sei");
	POKE(VIRQ, LOW_BYTE((uint16_t)&irqHandler));
	POKE(VIRQ+1, HIGH_BYTE(&irqHandler));
	POKE(INT_MASK_1,0xFF); //mask all except SOF interrupt
	POKE(INT_MASK_0,(0xFF) & (~INT00_VKY_SOF));
	POKE(INT_PEND_0,0xFF); //clear both pending interrupts
	POKE(INT_PEND_1,0xFF);
	
	POKE(SYS_CTRL_1,SYS_CTRL_TEXT_PG);
	POKE(0xC000,0x70);
	POKE(SYS_CTRL_1,MMU_IO_COLOR);
	POKE(0xC000, 0x0C);
	
	POKE(SYS_CTRL_1, 0);
	POKE(0xD000,1); //text overlay
	POKE(0xD001,0); //font mode 80x80
	asm("cli");
        }
return 0;}
