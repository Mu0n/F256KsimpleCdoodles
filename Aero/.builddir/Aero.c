#include "D:\F256\llvm-mos\code\Aero\.builddir\trampoline.h"

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
#define TIMER_NOTE_DELAY 30

#include "f256lib.h"
struct timer_t midiTimer, refTimer; //timer_t structure for setting timer through the kernel


uint8_t bassNotes[12] = {};
uint8_t bassDelays[12] = {};

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
void prgChange(uint8_t channel, uint8_t prg)
{
	POKE(MIDI_FIFO, 0xC0 | channel);
	POKE(MIDI_FIFO, prg);
}
void midiNoteOff(uint8_t channel, uint8_t note)
{
    POKE(MIDI_FIFO, 0x80 | channel);
	POKE(MIDI_FIFO, note);
    POKE(MIDI_FIFO, 0x4F);
}
void midiNoteOn(uint8_t channel, uint8_t note)
{
	POKE(MIDI_FIFO, 0x90 | channel);
	POKE(MIDI_FIFO, note);
	POKE(MIDI_FIFO, 0x4F);
}
void setup()
{
	textClear();
	textDefineForegroundColor(0,0xff,0xff,0xff);
	
	midiTimer.units = TIMER_FRAMES;
	midiTimer.absolute = TIMER_NOTE_DELAY;
    midiTimer.cookie = TIMER_NOTE_COOKIE;
	
	refTimer.units = TIMER_FRAMES;
	refTimer.absolute = TIMER_TEXT_DELAY;
	refTimer.cookie = TIMER_TEXT_COOKIE;
	
	setTimer(&midiTimer);
	setTimer(&refTimer);
	
	prgChange(0,33);
}


int main(int argc, char *argv[]) {
	uint16_t i, toDo;
	uint8_t bassCursor = 0;
	uint8_t recByte;
	bool startBass = false;
	bool startBeat = false;
	uint8_t prgInst = 0x30, note = 0x40;
	
	
	POKE(1,0);
	setup();
	
	prgChange(0x0A, 0);
	while(true)
        {
		if(!(PEEK(MIDI_CTRL) & 0x02)) //rx not empty
			{
				toDo = PEEKW(MIDI_RXD) & 0x0FFF; //discard top 4 bits of MIDI_RXD+1
				textGotoXY(0,0); printf("%02x  ",PEEK(0xDDA3) & 0x0F);
				textGotoXY(0,1); printf("%02x  ",PEEK(0xDDA4));
				textGotoXY(0,2);
				printf("                                 ");
				textGotoXY(0,2);
				for(i=0; i<toDo; i++)
				{
					recByte=PEEK(MIDI_FIFO);
					printf("%02x  ",recByte);
					POKE(MIDI_FIFO, recByte);
				}
			}
	
		kernelNextEvent();
        if(kernelEventData.type == kernelEvent(timer.EXPIRED))
            {
			switch(kernelEventData.timer.cookie)
				{
				case TIMER_TEXT_COOKIE:
					refTimer.absolute = getTimerAbsolute(TIMER_FRAMES) + TIMER_TEXT_DELAY;
					setTimer(&refTimer);
					break;
				case TIMER_NOTE_COOKIE:
					if(startBeat)
					{
						midiNoteOff(0x09, note);
						midiNoteOn(0x09, note);
						midiTimer.absolute = getTimerAbsolute(TIMER_FRAMES) + TIMER_NOTE_DELAY;
						setTimer(&midiTimer);
					}
					break;
				}
            }
		else if(kernelEventData.type == kernelEvent(key.PRESSED))
			{
				switch(kernelEventData.key.raw)
				{
					case 0xb6: //up
						if(prgInst < 127) {
							prgInst++;
							prgChange(0x09, prgInst);
						}
						break;
					case 0xb7: //down
						if(prgInst > 0) {
							prgInst--;
							prgChange(0x09, prgInst);
						}
						break;
					case 0xb8: //left
						if(note > 0) note--;
						break;
					case 0xb9: //right
						if(note < 127) note++;
						break;
					case 32: //space
						startBass = ~startBass;
						startBeat = !startBeat;
						midiTimer.absolute = getTimerAbsolute(TIMER_FRAMES) + TIMER_NOTE_DELAY;
						setTimer(&midiTimer);
						break;
				}
			}
	
        }
return 0;}
