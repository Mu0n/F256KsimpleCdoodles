#define F256LIB_IMPLEMENTATION
#include "f256lib.h"

#define MIDI_CTRL 0xDDA0
#define MIDI_OUT 0xDDA1
#define MIDI_RX_00_07 0xDDA2
#define MIDI_RX_08_10 0xDDA3

#define TIMER_FRAMES 0
#define TIMER_SECONDS 1

#define TIMER_NOTE_DELAY 30
#define TIMER_NOTE_COOKIE 0

struct timer_t midiTimer; //timer_t structure for setting timer through the kernel
uint8_t cScaleNotes[8]={40,42,44,45,47,49,51,52};
					   
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
  
void setPlayerInstrument(uint8_t choice)
{
	POKE(MIDI_OUT, 0xC1);
	POKE(MIDI_OUT, choice);
}
void setInstruments()
{
	POKE(MIDI_OUT,123);
	POKE(MIDI_OUT,0);
	POKE(MIDI_OUT,0xFF);
	//prep the MIDI organ instrument 19
	POKE(MIDI_OUT, 0xC0);
	POKE(MIDI_OUT, 00);
}

void midiNoteOff(uint8_t wantNote, uint8_t chan)
{
	//Send a Note_Off midi command for the previously ongoing note
    POKE(MIDI_OUT, 0x80 | chan);
	POKE(MIDI_OUT, wantNote);
    POKE(MIDI_OUT, 0x4F);
}
void midiNoteOn(uint8_t wantNote, uint8_t chan)
{
	//Send a Note_On midi command on channel 0		
	POKE(MIDI_OUT, 0x90 | chan);
	POKE(MIDI_OUT, wantNote);
	POKE(MIDI_OUT, 0x7F);
	//printf("%d ",wantNote);
	//keep track of that note so we can Note_Off it when needed
}

int main(int argc, char *argv[]) {
	uint8_t noteCursor=0, currentNote;
	bool isScaleActive = false;
	
	setInstruments();
	
	POKE(MMU_IO_CTRL,0);
	midiTimer.cookie = TIMER_NOTE_COOKIE;
	midiTimer.units = TIMER_FRAMES;

	printf("Press space to play a C scale with MIDI");
	while(true) 
			{
			kernelNextEvent();
			if(kernelEventData.type == kernelEvent(timer.EXPIRED))
				{
				switch(kernelEventData.timer.cookie)
					{
					case TIMER_NOTE_COOKIE:
						midiNoteOff(currentNote, 0);
						if(isScaleActive)
							{
							noteCursor++;
							if(noteCursor == 8)
								{
								isScaleActive = false;
								break;
								}
							}
							currentNote = cScaleNotes[noteCursor];
							midiNoteOn(currentNote, 0);
							
							midiTimer.absolute = getTimerAbsolute(TIMER_FRAMES) + TIMER_NOTE_DELAY;
							setTimer(&midiTimer);
						break;
					
					}	
				}				
			else if(kernelEventData.type == kernelEvent(key.PRESSED))
				{
					//the following line can be used to get keyboard codes
					//printf(" %d  ",kernelEventData.key.raw);
					switch(kernelEventData.key.raw)
					{						
						case 32: //space
							if(isScaleActive == false)
							{
								
							noteCursor=0;
							isScaleActive = true;
							currentNote = cScaleNotes[noteCursor];
							midiNoteOn(currentNote, 0);
							midiTimer.absolute = getTimerAbsolute(TIMER_FRAMES) + TIMER_NOTE_DELAY;
							setTimer(&midiTimer);
							}						
							break;
					}

					//printf("%d %d ",KeyCodesToMIDINotes[kernelEventData.key.raw],songNotes[noteCursor]);
				}
			
			}
return 0;}
}