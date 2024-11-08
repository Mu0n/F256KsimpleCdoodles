#include "D:\F256\llvm-mos\code\midiStuff\.builddir\trampoline.h"

#define F256LIB_IMPLEMENTATION

#define MIDI_CTRL 	   0xDDA0
#define MIDI_FIFO 	   0xDDA1
#define MIDI_RXD 	   0xDDA2
#define MIDI_RXD_COUNT 0xDDA3
#define MIDI_TXD       0xDDA4
#define MIDI_TXD_COUNT 0xDDA5

#define TIMER_FRAMES 0
#define TIMER_SECONDS 1

 //TIMER_TEXT for the 1-frame long text refresh timer for data display
 //TIMER_NOTE is for a midi note timer
#define TIMER_TEXT_COOKIE 0
#define TIMER_NOTE_COOKIE 1
#define TIMER_TEXT_DELAY 1
#define TIMER_NOTE_DELAY 2

#include "f256lib.h"
struct timer_t midiTimer, refTimer; //timer_t structure for setting timer through the kernel

uint16_t note = 0x36, oldnote; /*note is the current midi hex note code to send. oldnote keeps the previous one so it can be Note_off'ed away after the timer expires, or a new note is called*/
uint16_t prgInst = 0; /* program change value, the MIDI instrument number */

struct midi_uart {
	uint8_t status;
	uint8_t data;
	uint16_t bytes_in_rx;
	uint16_t bytes_in_tx;
} myMIDIsnapshot, *myMIDIptr;

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

void setup()
{
	textClear();
	textDefineForegroundColor(0,0xff,0xff,0xff);
    textGotoXY(0,0); textPrint("running a MIDI test\nLEFT/RIGHT changes note pitch; UP/DOWN changes instrument; Space to play a note");
	textGotoXY(0,3); textPrint("0xDDA0");
	textGotoXY(0,4); textPrint("0xDDA1");
	textGotoXY(0,5); textPrint("0xDDA2");
	textGotoXY(0,6); textPrint("0xDDA3");
	textGotoXY(0,7); textPrint("0xDDA4");
	textGotoXY(0,8); textPrint("0xDDA5");
	
	textGotoXY(0,10); textPrint("Instrument: ");
	textGotoXY(0,11); textPrint("Note: ");
	
	midiTimer.units = TIMER_SECONDS;
	midiTimer.absolute = TIMER_NOTE_DELAY;
    midiTimer.cookie = TIMER_NOTE_COOKIE;
	
	refTimer.units = TIMER_FRAMES;
	refTimer.absolute = TIMER_TEXT_DELAY;
	refTimer.cookie = TIMER_TEXT_COOKIE;
	
	setTimer(&midiTimer);
	setTimer(&refTimer);
	
	POKE(MIDI_FIFO, 0xC0);
	POKE(MIDI_FIFO, 0);
}

void refreshPrints()
{
	
	textGotoXY(10,3); printf("%02x  ",PEEK(0xDDA0));
	//textGotoXY(10,4); printf("%02x  ",PEEK(0xDDA1));
	POKE(0xDDA1, PEEK(0xDDA1));
	
	textGotoXY(10,5); printf("%02x  ",PEEK(0xDDA2));
	textGotoXY(10,6); printf("%02x  ",PEEK(0xDDA3));
	textGotoXY(10,7); printf("%02x  ",PEEK(0xDDA4));
	textGotoXY(10,8); printf("%02x  ",PEEK(0xDDA5));
	
	textGotoXY(13,10); printf(" %d",prgInst);
	textGotoXY(13,11); printf(" %d",note);
}

void midiNoteOff()
{
	//Send a Note_Off midi command for the previously ongoing note
    POKE(MIDI_FIFO, 0x80);
	POKE(MIDI_FIFO, oldnote);
    POKE(MIDI_FIFO, 0x4F);
}
void midiNoteOn()
{
	//Send a Note_On midi command on channel 0
	POKE(MIDI_FIFO, 0x90);
	POKE(MIDI_FIFO, note);
	POKE(MIDI_FIFO, 0x4F);
	//keep track of that note so we can Note_Off it when needed
	oldnote = note;
}
void commandNote()
{
	uint8_t result;
	midiNoteOff();
	//Prepare the next note timer
	result = getTimerAbsolute(TIMER_SECONDS);
	//the following line can be used to check absolute timer delays
	//printf(" result: %d %d       ",result , result + TIMER_NOTE_DELAY);
	midiTimer.absolute = result + TIMER_NOTE_DELAY;
	setTimer(&midiTimer);
	midiNoteOn();
}
void prgChange(bool wantUpOrDown)
{
	if(wantUpOrDown) prgInst++;
	else prgInst--;
	POKE(MIDI_FIFO, 0xC0);
	POKE(MIDI_FIFO, prgInst);
}
int main(int argc, char *argv[]) {
	
	setup();
	POKE(1,0);
	myMIDIptr = (struct midi_uart *) MIDI_CTRL;
    while(true)
        {
			/*
		if(myMIDIptr->status != 0x02)
		{
			while(myMIDIptr->bytes_in_rx > 0)
			{
				POKE(MIDI_FIFO, myMIDIptr->data);
			}
	
		}
		*/
	
	
	
	/*#
define MIDI_CTRL 	   0xDDA0
#define MIDI_FIFO 	   0xDDA1
#define MIDI_RXD 	   0xDDA2
#define MIDI_RXD_COUNT 0xDDA3
#define MIDI_TXD       0xDDA4
#define MIDI_TXD_COUNT 0xDDA5
*/

	
		kernelNextEvent();
        if(kernelEventData.type == kernelEvent(timer.EXPIRED))
            {
			switch(kernelEventData.timer.cookie)
				{
				case TIMER_TEXT_COOKIE:
					refreshPrints();
					refTimer.absolute = getTimerAbsolute(TIMER_FRAMES) + TIMER_TEXT_DELAY;
					setTimer(&refTimer);
					break;
				case TIMER_NOTE_COOKIE:
					midiNoteOff();
					break;
				}
            }
		else if(kernelEventData.type == kernelEvent(key.PRESSED))
			{
				switch(kernelEventData.key.raw)
				{
					case 0xb6: //up
						if(prgInst < 127) prgChange(true);
						break;
					case 0xb7: //down
						if(prgInst > 0) prgChange(false);
						break;
					case 0xb8: //left
						if(note > 0) note--;
						break;
					case 0xb9: //right
						if(note < 127) note++;
						break;
					case 32: //space
						commandNote();
						break;
				}
				//the following line can be used to get keyboard codes
				//printf(" %d",kernelEventData.key.raw);
			}
	
        }
return 0;}
