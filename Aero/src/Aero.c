#define F256LIB_IMPLEMENTATION

#define MIDI_CTRL 	   0xDDA0
#define MIDI_FIFO 	   0xDDA1
#define MIDI_RXD 	   0xDDA2
#define MIDI_RXD_COUNT 0xDDA3
#define MIDI_TXD       0xDDA4
#define MIDI_TXD_COUNT 0xDDA5


#define BMP_DAFT       0x10000
#define BMP_PIANO	   0x30000

#define TIMER_FRAMES 0
#define TIMER_SECONDS 1

 //TIMER_TEXT for the 1-frame long text refresh timer for data display
 //TIMER_NOTE is for a midi note timer
#define TIMER_TEXT_COOKIE 0
#define TIMER_NOTE_COOKIE 1
#define TIMER_BASS_COOKIE 2
#define TIMER_SNARE_COOKIE 3
#define TIMER_KICK_COOKIE 4

#define TIMER_TEXT_DELAY 1
#define TIMER_NOTE_DELAY 30



#include "f256lib.h"


EMBED(daft1, "../assets/xaa", 0x10000);
EMBED(daft2, "../assets/xab", 0x18000);
EMBED(daft3, "../assets/xac", 0x20000);
EMBED(palpiano, "../assets/pian.pal", 0x28000);
EMBED(paldaft, "../assets/daft.pal", 0x28400);
EMBED(pia1, "../assets/piaa", 0x30000);
EMBED(pia2, "../assets/piab", 0x38000);
EMBED(pia3, "../assets/piac", 0x40000);

struct timer_t midiTimer, refTimer, bassTimer, snareTimer, kickTimer; //timer_t structure for setting timer through the kernel


uint8_t bassNotes[12] = {0x00,0x1F,0x24,0x22,0x24,0x00,0x00,0x1F,0x20,0x20,0x1D,0x00};
uint8_t bassDelays[12] =  {15,  15,  23,  22,  15,  30,  15,  15,  23,  22,  15,  30};
uint8_t snareNotes[4] = {0x00, 0x26, 0x00, 0x26};
uint8_t snareDelays[4] = {30, 30, 30, 30};
uint8_t kickNotes[5] = {0x00, 0x24, 0x24, 0x24,0x00};
uint8_t kickDelays[5] = {30, 23, 23, 22, 22};
bool verbose = false; //used to display debugging text
bool noteColors[88]={1,0,1, 1,0,1,0,1, 1,0,1,0,1,0,1, 1,0,1,0,1, 1,0,1,0,1,0,1, 1,0,1,0,1, 1,0,1,0,1,0,1, 1,0,1,0,1, 1,0,1,0,1,0,1, 1,0,1,0,1, 1,0,1,0,1,0,1, 1,0,1,0,1, 1,0,1,0,1,0,1, 1,0,1,0,1, 1,0,1,0,1,0,1, 1};

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

void midiNoteOnSoft(uint8_t channel, uint8_t note)
{
	POKE(MIDI_FIFO, 0x90 | channel);
	POKE(MIDI_FIFO, note);
	POKE(MIDI_FIFO, 0x2F);
}

void setInstruments()
{

	POKE(MIDI_FIFO,123);
	POKE(MIDI_FIFO,0);
	POKE(MIDI_FIFO,0xFF);
	
	prgChange(0,33);
	prgChange(0x01, 29); //distorted guitar on Channel 0x01
	prgChange(0x02, 38); //lead bass on Channel 0x02 
	prgChange(0x03, 87); //saw wave on Channel 0x03 
	
}


void setup()
{
	uint16_t c;
	
	textClear();
	textDefineForegroundColor(0,0xff,0xff,0xff);
	
	POKE(MMU_IO_CTRL, 0x00);
	POKE(VKY_MSTR_CTRL_0, 0x0E); //graphics and tile enabled
	POKE(VKY_MSTR_CTRL_1, 0x10); //320x240 at 60 Hz; double height text
	POKE(VKY_LAYER_CTRL_0, 0x10); //bitmap 0 in layer 0, bitmap 1 in layer 1
	POKE(VKY_LAYER_CTRL_1, 0x02); //bitmap 0 in layer 0, bitmap 1 in layer 1
	
	POKE(MMU_IO_CTRL,1);  //MMU I/O to page 1
	//prep to copy over the palette to the CLUT
	for(c=0;c<1023;c++) 
	{
		POKE(VKY_GR_CLUT_0+c, FAR_PEEK(0x28000+c)); //palette for piano
		POKE(VKY_GR_CLUT_1+c, FAR_PEEK(0x28400+c)); //palette for daft punks
	}
	
	
	POKE(MMU_IO_CTRL,0);
	
	//piano bitmap at layer 0	
	bitmapSetActive(0);
	bitmapSetAddress(0,BMP_PIANO);
	bitmapSetVisible(0,true);
	bitmapSetCLUT(0);
	
	//daft bunk bitmap at layer 1
	bitmapSetActive(1);
	bitmapSetAddress(1,BMP_DAFT);
	bitmapSetVisible(1,true);
	bitmapSetCLUT(1);
	
	bitmapSetActive(2);
	bitmapSetVisible(2,false);
	
	
	bitmapSetActive(0);
	
	midiTimer.units = TIMER_FRAMES;
	midiTimer.absolute = TIMER_NOTE_DELAY;
    midiTimer.cookie = TIMER_NOTE_COOKIE;
	
	refTimer.units = TIMER_FRAMES;
	refTimer.absolute = TIMER_TEXT_DELAY;
	refTimer.cookie = TIMER_TEXT_COOKIE;
	
	bassTimer.units = TIMER_FRAMES;
	bassTimer.absolute = 0;
	bassTimer.cookie = TIMER_BASS_COOKIE;
	
	snareTimer.units = TIMER_FRAMES;
	snareTimer.absolute = 0;
	snareTimer.cookie = TIMER_SNARE_COOKIE;
	
	kickTimer.units = TIMER_FRAMES;
	kickTimer.absolute = 0;
	kickTimer.cookie = TIMER_KICK_COOKIE;
	
	setTimer(&midiTimer);
	setTimer(&refTimer);
	setTimer(&bassTimer);
	
	setInstruments();
	
}


int main(int argc, char *argv[]) {
	uint16_t i, toDo;
	uint8_t bassIndex = 0, snareIndex = 0, kickIndex = 0;
	uint8_t recByte, detectedNote, detectedColor;
	bool startBass = false;
	bool startBeat = false;
	bool startSnare = false;
	bool startKick = false;
	bool nextIsNote = false; //detect a 0x9? or 0x8? command, the next is a note byte, used for coloring the keyboard note-rects
	bool isHit = false; // true is hit, false is released
	uint8_t prgInst = 0x30, note = 0x23;
	
	
	POKE(1,0);
	setup();
	

	while(true) 
        {
		if(!(PEEK(MIDI_CTRL) & 0x02)) //rx not empty
			{
				toDo = PEEKW(MIDI_RXD) & 0x0FFF; //discard top 4 bits of MIDI_RXD+1
				textGotoXY(0,0); printf("%02x  ",PEEK(0xDDA3) & 0x0F);
				textGotoXY(0,1); printf("%02x  ",PEEK(0xDDA4));
				textGotoXY(0,2); 
				if(verbose)printf("                                 ");
				textGotoXY(0,2); 
				for(i=0; i<toDo; i++)
				{
					recByte=PEEK(MIDI_FIFO);
					if(verbose)printf("%02x  ",recByte);
					
					if(nextIsNote)
					{
						detectedNote = recByte-0x14;
						if(isHit) graphicsDefineColor(0, detectedNote,0xFF,0x00,0xFF); //paint it hit
						else {
							detectedColor = noteColors[detectedNote-1]?0xFF:0x00;
							graphicsDefineColor(0, detectedNote,detectedColor,detectedColor,detectedColor); //swap back the original color
						}

						nextIsNote = false;
					}
					if(recByte == 0x90) {
						recByte = 0x91;
						nextIsNote = true;
						isHit=true;
					}
					if(recByte == 0x80) {
						recByte = 0x81;
						nextIsNote = true;
						isHit=false;
						}
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
				case TIMER_SNARE_COOKIE:
					if(startSnare)
						{
						midiNoteOff(0x09, snareNotes[snareIndex]);
						snareIndex++;
						if(snareIndex==4) snareIndex=0;
						midiNoteOn(0x09, snareNotes[snareIndex]);	
						snareTimer.absolute = getTimerAbsolute(TIMER_FRAMES) + snareDelays[snareIndex];
						setTimer(&snareTimer); 
						}
					break;
				case TIMER_KICK_COOKIE:
					if(startKick)
						{
						midiNoteOff(0x09, kickNotes[kickIndex]);
						kickIndex++;
						if(kickIndex==5) kickIndex=0;
						midiNoteOnSoft(0x09, kickNotes[kickIndex]);	
						kickTimer.absolute = getTimerAbsolute(TIMER_FRAMES) + kickDelays[kickIndex];
						setTimer(&kickTimer); 
						}
					break;
				case TIMER_BASS_COOKIE:
					if(startBass)
					{
						midiNoteOff(0x02, bassNotes[bassIndex]);
						midiNoteOff(0x03, bassNotes[bassIndex]);
						bassIndex++;
						if(bassIndex==12) bassIndex = 0; //loop back
						midiNoteOnSoft(0x02, bassNotes[bassIndex]);	
						midiNoteOnSoft(0x03, bassNotes[bassIndex]);	
						bassTimer.absolute = getTimerAbsolute(TIMER_FRAMES) + bassDelays[bassIndex];
						setTimer(&bassTimer);
					}
					break;
				case TIMER_NOTE_COOKIE:/*
					if(startBeat)
					{
						midiNoteOff(0x09, note);	
						midiNoteOn(0x09, note);	
						midiTimer.absolute = getTimerAbsolute(TIMER_FRAMES) + TIMER_NOTE_DELAY;
						setTimer(&midiTimer);
					}*/
					break;
				}	
            }				
		else if(kernelEventData.type == kernelEvent(key.PRESSED))
			{
				switch(kernelEventData.key.raw)
				{
					case 146: // top left backspace, meant as reset
						setInstruments();											
						break;
					case 0xb6: //up
						if(prgInst < 127) {
							prgInst++;
							//prgChange(0x09, prgInst); 
							prgChange(0x01,prgInst);
							textGotoXY(0,11);
							printf("%02x ",note);
						}
						break;
					case 0xb7: //down
						if(prgInst > 0) {
							prgInst--;
							//prgChange(0x09, prgInst); 
							prgChange(0x01,prgInst);
							textGotoXY(0,11);
							printf("%02x ",note);
						}
						break;
					case 0xb8: //left
						if(note > 0) note--; //percussion change
						textGotoXY(0,10);
						printf("%02x ",note); 
						break;
					case 0xb9: //right
						if(note < 127) note++; //percussion change
						textGotoXY(0,10);
						printf("%02x ",note);
						break;
					case 32: //space
						startBass = !startBass;
						//startBeat = !startBeat;
						startSnare = !startSnare;
						startKick = !startKick;
						
						midiTimer.absolute = getTimerAbsolute(TIMER_FRAMES) + TIMER_NOTE_DELAY;
						setTimer(&midiTimer);
						
						if(startBass == true) bassIndex = 0;
						if(startSnare == true) snareIndex = 0;
						if(startKick == true) kickIndex = 0;
						bassTimer.absolute = getTimerAbsolute(TIMER_FRAMES) + bassDelays[bassIndex];
						setTimer(&bassTimer);
						snareTimer.absolute = getTimerAbsolute(TIMER_FRAMES) + snareDelays[snareIndex];
						setTimer(&snareTimer);
						kickTimer.absolute = getTimerAbsolute(TIMER_FRAMES) + kickDelays[kickIndex];
						setTimer(&kickTimer);
						break;
				}
			}
			
        }
return 0;}
}