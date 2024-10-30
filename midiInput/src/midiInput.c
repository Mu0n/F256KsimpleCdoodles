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

int main(int argc, char *argv[]) {
	bool disa = false;
	setup();
	POKE(1,0);
    while(true) 
        {
		
        }
return 0;}
