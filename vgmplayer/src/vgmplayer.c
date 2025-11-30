#define F256LIB_IMPLEMENTATION
#include "f256lib.h"

#include "../src/muUtils.h" //contains helper functions I often use
#include "../src/muopl3.h" //opl3 chip routines
#include "../src/muvgmplay.h" //used to play vgms
#include "../src/muFilePicker.h"  //file picker I use everywhere
#include "../src/muTimer0Int.h"  //timer0 routines with an interval of max 2/3 of a second
#include "../src/mulcd.h"  //K2 LCD
#include "../src/muMIDIin.h" //midi in keyboard
#include "../src/muMidi.h"
#include "../src/mudispatch.h"

#include <stdlib.h>

#define INSTR_LINE 28
#define LCDBIN 0x30000

void textUI(void);
void eraseLine(uint8_t);
void snoopLoop(midiInData *);
 

EMBED(pianopal, "../assets/piano.pal", 0x20000); //1kb
EMBED(keys, "../assets/piano.raw", 0x20400); //

//prototypes:
void setup(void);
int8_t dealPressedKey(void);

// UNUSED?
/*
uint32_t convert_samples_to_timer_ticks(uint32_t samples) {
    // Timer0 max = 0xFFFFFF = 16777215 ticks = 2/3 sec
    // So: ticks_per_sample = (2/3 sec / 16777215) * vgm_rate
    // Simplified:
    return (samples * 0x66666) ;  // 667000 µs = 2/3 sec
}
*/

int8_t dealPressedKey()
{
kernelNextEvent();
if(kernelEventData.type == kernelEvent(key.PRESSED))
	{
		if(kernelEventData.key.raw == 0x83) //F3
			{
				eraseLine(0);eraseLine(1);
				return 1;
				
			}
		if(kernelEventData.key.raw == 146) //esc
			{
			opl3_quietAll();
			return -1;
			}
		if(kernelEventData.key.raw == 0x20) //space for pause
			{
			opl3_quietAll();		
			return -2;
			}
	}
return 0;
}



void instructions()
{
textEnableBackgroundColors(true);
textSetColor(13,1);
textGotoXY(0,0);printf("OPL3 Snooper                               ");
textGotoXY(0,1);printf("     Created by Mu0n, November 2025 v0.3   ");
}
void textUI()
{	

textGotoXY(0,INSTR_LINE);
textSetColor(7,6);printf("[ESC]");
textSetColor(15,0);printf(" Quit ");
textSetColor(7,6);printf("[F3]");
textSetColor(15,0);printf(" Load ");
textSetColor(7,6);printf("[Space]");
textSetColor(15,0);printf(" Pause and Snoop!");
}

void eraseLine(uint8_t line)
{	
textGotoXY(0,line);printf("                                                                                ");
}




void setup()
{
POKE(MMU_IO_CTRL, 0x00);
// XXX GAMMA  SPRITE   TILE  | BITMAP  GRAPH  OVRLY  TEXT
POKE(VKY_MSTR_CTRL_0, 0b00001111); //sprite,graph,overlay,text
// XXX XXX  FON_SET FON_OVLY | MON_SLP DBL_Y  DBL_X  CLK_70
POKE(VKY_MSTR_CTRL_1, 0b00000100); //font overlay, double height text, 320x240 at 60 Hz;
POKE(VKY_LAYER_CTRL_0, 0b00010000); //bitmap 0 in layer 0, bitmap 1 in layer 1
POKE(0xD00D,0x00); //force black graphics background
POKE(0xD00E,0x00);
POKE(0xD00F,0x00);	


POKE(MMU_IO_CTRL,1); //MMU I/O to page 1
for(uint16_t c=0;c<1023;c++) 
{
	POKE(VKY_GR_CLUT_0+c, FAR_PEEK(0x20000+c)); //palette for piano
}
POKE(MMU_IO_CTRL,0); //MMU I/O to page 0
	
bitmapReset();
bitmapSetAddress(0,0x20400);
bitmapSetVisible(0,true);
initFPR();

//resetGlobals(gPtr);

//displayImage(LCDBIN);

}

void snoopLoop(midiInData *gM) //reached when in paused mode
{
uint8_t escape = 1;
while(escape)
	{
	dealMIDIIn(gM);
	kernelNextEvent();
	
	if(kernelEventData.type == kernelEvent(key.PRESSED) && kernelEventData.key.raw == 0x20) escape = 0;
	
			
	}
}

int main(int argc, char *argv[]) {
FILE *theFile;
uint8_t exitFlag = 0;
int checkKey = 0;
midiInData gMID;
uint8_t startByte  =0;

setup();
resetMID(&gMID);
opl3_setInstrumentAllChannels(11);

while(exitFlag == 0)
	{
	opl3_initialize();
	instructions();
	if(getTheFile(name)!=0) return 0;
	theFile = load_VGM_file(name); //gets a FILE opened and ready to use
	startByte = checkVGMHeader(theFile);
	fseek(theFile, (uint32_t)startByte, SEEK_SET);
	
	//info on screen
	eraseLine(3);
	textGotoXY(0,3);textSetColor(7,0);printf("Playing: %s",name);
	textUI();
		
	comeRightTrough = true; //to kickstart it
	while(true)//main play and input loop
		{
			if(playback(theFile) == -1)
				{
				fileClose(theFile);
				break;
				}			
			checkKey = dealPressedKey();
			if(checkKey == 1) 
				{
				fileClose(theFile);
				break;
				}
			if(checkKey == -1)
				{
				fileClose(theFile);
				exitFlag = 1;
				break;
				}	
			if(checkKey == -2) //pause is hit
				{
				snoopLoop(&gMID);
				}	
		}
	}

opl3_initialize();
return 0;}

}