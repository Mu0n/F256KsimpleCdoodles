#include "D:\F256\llvm-mos\code\psidplayer\.builddir\trampoline.h"

#define F256LIB_IMPLEMENTATION
#include "f256lib.h"

#include "../src/muUtils.h" //contains helper functions I often use
#include "../src/musid.h" //contains helper functions I often use
#include "../src/muFilePicker.h"  //contains basic MIDI functions I often use
#include "../src/muTimer0Int.h"  //contains basic MIDI functions I often use
#include <stdlib.h>


#define TIMER_FRAMES 0
#define TIMER_SECONDS 1
#define TIMER_SID_DELAY 1
#define TIMER_SID_COOKIE 0

#define INSTR_LINE 24

FILE *load_sid_file(void);
uint8_t getTheFile(void);
uint8_t playback(FILE *);
void textUI(void);
void eraseLine(uint8_t);

// Global playback variables
struct timer_t sidTimer;
filePickRecord fpr;
char name[100];


void swapBlock5(void)
{
	POKE(0x0000, 0x91); //edit 3 act 3
	POKE(0x0009, 0x33); //point bank 4 to block $33 of RAM
	
	POKE(0x8123, 0xAB); //poke something
}
//Opens the std MIDI file
FILE *load_sid_file() {
	FILE *theSIDfile;
	
	theSIDfile = fileOpen(name,"r"); // open file in read mode
	if(theSIDfile == NULL) {
		return NULL;
		}
	return theSIDfile;

}

uint8_t playback(FILE *theSIDfile)
{
	uint8_t buffer[25];
	size_t bytesRead = 0;
	uint8_t exitFlag = 0;
	eraseLine(0);eraseLine(1);
	textGotoXY(0,0);printf("Playing: %s",name);
	textUI();
	
	sidTimer.units = TIMER_FRAMES;
	sidTimer.cookie = TIMER_SID_COOKIE;
	sidTimer.absolute = getTimerAbsolute(TIMER_FRAMES) + TIMER_SID_DELAY;
	setTimer(&sidTimer);
	
	setTimer0(0x7aE15); //0.02 s, exactly 3% of the range, to make it 50Hz
	
	while(!exitFlag)
	{
		kernelNextEvent();
		if(kernelEventData.type == kernelEvent(key.PRESSED))
		{
			if(kernelEventData.key.raw == 0x83) //F3
				{
					eraseLine(0);eraseLine(39);
					return 0;
	
				}
			if(kernelEventData.key.raw == 146) //esc
				{
				clearSIDRegisters();
				return 1;
				}
		}/*
		if(kernelEventData.type == kernelEvent(timer.EXPIRED))
			{
			switch(kernelEventData.timer.cookie)
				{
				case TIMER_SID_COOKIE:
					bytesRead = fileRead(buffer, sizeof(uint8_t), 25, theSIDfile );
					if(bytesRead < 25) exitFlag = 1;
	
					for(uint8_t i=0;i<25;i++)
						{
						POKE(SID1+i, buffer[i]);
						}
					sidTimer.absolute = getTimerAbsolute(TIMER_FRAMES) + TIMER_SID_DELAY;
					setTimer(&sidTimer);
					break;

				}
			}
			*/
		if(PEEK(INT_PENDING_0)&0x10) //when the timer0 delay is up, go here
			{
			POKE(INT_PENDING_0,0x10); //clear the timer0 delay
			//play the next chunk of the midi file, might deal with multiple 0 delay stuff
			bytesRead = fileRead(buffer, sizeof(uint8_t), 25, theSIDfile );
			if(bytesRead < 25) exitFlag = 1;
	
			for(uint8_t i=0;i<25;i++)
				{
				POKE(SID1+i, buffer[i]);
				}
			setTimer0(0x7aE15);
			}
	
	}

	eraseLine(0);eraseLine(INSTR_LINE);
	return 0;
}

void instructions()
{
textSetColor(15,0);
textGotoXY(0,0);printf("Raw PSID player: plays files from the HVS Collection converted with zigreSID");
textGotoXY(0,1);printf("     Created by Mu0n, sept 2025 v0.1");
}
void textUI()
{
textGotoXY(0,INSTR_LINE);printf("[ESC: Quit] [F3: Load]");
}

void eraseLine(uint8_t line)
{
textGotoXY(0,line);printf("                                                                                ");
}
uint8_t getTheFile() //job is to get a string containing the filename including directory
{
	
//check if the midi directory is here, if not, target the root
	char *dirOpenResult = fileOpenDir("sid");
	if(dirOpenResult != NULL)
	{
		strncpy(fpr.currentPath, "sid", MAX_PATH_LEN);
		fileCloseDir(dirOpenResult);
	}
	else strncpy(fpr.currentPath, "0:", MAX_PATH_LEN);


	uint8_t wantsQuit = filePickModal(&fpr, DIRECTORY_X, DIRECTORY_Y, "raw", "", "", "", true);
	if(wantsQuit==1) return 1;
	sprintf(name, "%s%s%s", fpr.currentPath,"/", fpr.selectedFile);
	return 0;
}


int main(int argc, char *argv[]) {
FILE *theFile;
uint8_t exitFlag = 0;

POKE(MMU_IO_CTRL, 0x00);
// XXX GAMMA  SPRITE   TILE  | BITMAP  GRAPH  OVRLY  TEXT
POKE(VKY_MSTR_CTRL_0, 0b00001111); //sprite,graph,overlay,text
// XXX XXX  FON_SET FON_OVLY | MON_SLP DBL_Y  DBL_X  CLK_70
POKE(VKY_MSTR_CTRL_1, 0b00010100); //font overlay, double height text, 320x240 at 60 Hz;
POKE(VKY_LAYER_CTRL_0, 0b00010000); //bitmap 0 in layer 0, bitmap 1 in layer 1
POKE(0xD00D,0x00); //force black graphics background
POKE(0xD00E,0x00);
POKE(0xD00F,0x00);

setMonoSID();

swapBlock5();
hitspace();

while(exitFlag == 0)
	{
	clearSIDRegisters();
	instructions();
	if(getTheFile()!=0) return 0;
	theFile = load_sid_file(); //gets a FILE opened and ready to use
	switch(playback(theFile))
		{
			case 0: //success playing or switch file name
				break;
			case 1: //wants to quit
				fileClose(theFile);
				return 0;
		}
	fileClose(theFile);
	}

return 0;}

