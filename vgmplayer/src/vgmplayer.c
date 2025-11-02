#define F256LIB_IMPLEMENTATION
#include "f256lib.h"

#include "../src/muUtils.h" //contains helper functions I often use
#include "../src/muopl3.h" //contains helper functions I often use
#include "../src/muFilePicker.h"  //contains basic MIDI functions I often use
#include "../src/muTimer0Int.h"  //contains basic MIDI functions I often use
#include <stdlib.h>


#define TIMER_FRAMES 0
#define TIMER_SECONDS 1
#define TIMER_VGM_DELAY 1
#define TIMER_VGM_COOKIE 0

#define INSTR_LINE 24

FILE *load_VGM_file(void);
uint8_t getTheFile(void);
void checkVGMHeader(FILE *);
uint8_t playback(FILE *);
void textUI(void);
void eraseLine(uint8_t);

// Global playback variables
struct timer_t VGMTimer;
filePickRecord fpr;
char name[100];

uint32_t convert_samples_to_timer_ticks(uint32_t samples) {
    // Timer0 max = 0xFFFFFF = 16777215 ticks = 2/3 sec
    // So: ticks_per_sample = (2/3 sec / 16777215) * vgm_rate
    // Simplified:
    return (samples * 0x66666) ;  // 667000 µs = 2/3 sec
}
//Opens the std MIDI file
FILE *load_VGM_file() {
	FILE *theVGMfile;
	
	theVGMfile = fileOpen(name,"r"); // open file in read mode
	if(theVGMfile == NULL) {
		return NULL;
		}
	return theVGMfile;

}

void checkVGMHeader(FILE *theVGMfile)
{
	uint8_t buffer[16];
	size_t bytesRead = 0;
	textGotoXY(0,3);
	for(uint8_t i = 0; i<8; i++)
	{	
	bytesRead = fileRead(buffer, sizeof(uint8_t), 16, theVGMfile );
		if(bytesRead < 4) return;
	for(uint8_t j = 0;j<16;j++) printf("%02x ", buffer[j]);
	printf("\n");
	}		

}


uint8_t playback(FILE *theVGMfile)
{
	uint8_t nextRead, countRead=0;
	uint8_t reg, val;
	uint8_t hi, lo;
	uint8_t exitFlag = 0;
	eraseLine(0);eraseLine(1);
	textGotoXY(0,0);printf("Playing: %s",name);
	textUI();
	
	VGMTimer.units = TIMER_FRAMES;
	VGMTimer.cookie = TIMER_VGM_COOKIE;
	VGMTimer.absolute = getTimerAbsolute(TIMER_FRAMES) + TIMER_VGM_DELAY;
	//setTimer(&VGMTimer);
	
	
	setTimer0(0x1); //0.02 s, exactly 3% of the range, to make it 60Hz
	
	while(!exitFlag)
	{
		hi=0;lo=0;
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
				opl3_quietAll();
				return 1;
				}
		}/*
		if(kernelEventData.type == kernelEvent(timer.EXPIRED))
			{		
			switch(kernelEventData.timer.cookie)
				{				
				case TIMER_VGM_COOKIE:
					bytesRead = fileRead(buffer, sizeof(uint8_t), 25, theVGMfile );
					if(bytesRead < 25) exitFlag = 1;
					
					for(uint8_t i=0;i<25;i++) 
						{
						POKE(VGM1+i, buffer[i]);
						}
					VGMTimer.absolute = getTimerAbsolute(TIMER_FRAMES) + TIMER_VGM_DELAY;
					setTimer(&VGMTimer);
					break;

				}
			}
			*/
			/*
		if(PEEK(INT_PENDING_0)&0x10) //when the timer0 delay is up, go here
			{
			POKE(INT_PENDING_0,0x10); //clear the timer0 delay
			//play the next chunk of the midi file, might deal with multiple 0 delay stuff
			bytesRead = fileRead(buffer, sizeof(uint8_t), 25, theVGMfile );
			if(bytesRead < 25) exitFlag = 1;
		

			setTimer0(0x7aE15);
			}
*/

		if(PEEK(INT_PENDING_0)&0x10) //when the timer0 delay is up, go here
			{
			POKE(INT_PENDING_0,0x10); //clear the timer0 delay
				
			countRead = fileRead(&nextRead, sizeof(uint8_t), 1, theVGMfile);
						
			if (countRead == 1) {
				switch (nextRead) {
					case 0x5E:  // YMF262 write port 0
						fileRead(&reg, sizeof(uint8_t), 1, theVGMfile);
						fileRead(&val, sizeof(uint8_t), 1, theVGMfile);
						opl3_write(reg, val);
						setTimer0(1);
						break;
						
					case 0x5F:  // YMF262 write port 1
						fileRead(&reg, sizeof(uint8_t), 1, theVGMfile);
						fileRead(&val, sizeof(uint8_t), 1, theVGMfile);
						opl3_write((0x100 | (uint16_t)reg), val);
						setTimer0(1);
						break;
						
					case 0x61:  // Wait n samples
						fileRead(&lo, sizeof(uint8_t), 1, theVGMfile);
						fileRead(&hi, sizeof(uint8_t), 1, theVGMfile);
						setTimer0(((((uint32_t)hi)<<8)|((uint32_t)lo))*(uint32_t)0x23A);
						break;

					case 0x62: 
						setTimer0(0x66666);   // 60Hz
						break;
						
					case 0x63: 
						setTimer0(0x7aE15);  // 50Hz
						break;
					case 0x66: // End of sound data
						exitFlag = 1;
						break;  

					case 0x31: //skip one data byte commands
					case 0x4F:
					case 0x50:
						fileRead(&reg, sizeof(uint8_t), 1, theVGMfile);
						setTimer0(1);
						break;
						
					case 0x40: //skip two data bytes commands
					case 0x51 ... 0x5D:
					case 0xA0:
					case 0xB0 ... 0xC8:
						fileRead(&reg, sizeof(uint8_t), 1, theVGMfile);
						fileRead(&reg, sizeof(uint8_t), 1, theVGMfile);
						setTimer0(1);
						break;
						
					case 0xD0 ... 0xD6: //skip 3 data bytes commands
						fileRead(&reg, sizeof(uint8_t), 1, theVGMfile);
						fileRead(&reg, sizeof(uint8_t), 1, theVGMfile);
						fileRead(&reg, sizeof(uint8_t), 1, theVGMfile);
						setTimer0(1);
						break;
					default:
						setTimer0(1);
						break;
					}
				}
			else return 0;
			}

	}			

	eraseLine(0);eraseLine(INSTR_LINE);
	return 0;
}

void instructions()
{
textSetColor(15,0);
textGotoXY(0,0);printf("Mu's VGM player");
textGotoXY(0,1);printf("     Created by Mu0n, oct 2025 v0.1");
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
	
//check if the vgm directory is here, if not, target the root
	char *dirOpenResult = fileOpenDir("vgm");
	if(dirOpenResult != NULL) 
	{
		strncpy(fpr.currentPath, "vgm", MAX_PATH_LEN);
		fileCloseDir(dirOpenResult);
	}
	else strncpy(fpr.currentPath, "0:", MAX_PATH_LEN);


	uint8_t wantsQuit = filePickModal(&fpr, DIRECTORY_X, DIRECTORY_Y, "vgm", "", "", "", true);
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

opl3_initialize();


while(exitFlag == 0)
	{
	opl3_quietAll();
	instructions();
	if(getTheFile()!=0) return 0;
	theFile = load_VGM_file(); //gets a FILE opened and ready to use
	checkVGMHeader(theFile);
	
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

}