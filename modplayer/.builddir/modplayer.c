#include "D:\F256\llvm-mos\code\modplayer\.builddir\trampoline.h"

#define F256LIB_IMPLEMENTATION
#include "f256lib.h"

#include "../src/muUtils.h" //contains helper functions I often use
#include "../src/muMODplay.h" //used to play vgms
#include "../src/muFilePicker.h"  //file picker I use everywhere
#include "../src/muTimer0Int.h"  //timer0 routines with an interval of max 2/3 of a second
#include "../src/mulcd.h"  //K2 LCD
#include "../src/textUI.h"

#include <stdio.h>
#include <stdlib.h>

#define LCDBIN 0x40000
void textUI(void);
void eraseLine(uint8_t);
void snoopLoop();

EMBED(pianopal, "../assets/piano.pal", 0x10000); //1kb
EMBED(keys, "../assets/piano.raw", 0x10400); //
EMBED(lcdimg, "../assets/opl3snoop.bin", 0x40000); //
//midiInData gMID;
FILE *theFile;

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
				eraseLine(0);eraseLine(1);eraseLine(2);eraseLine(3);eraseLine(39);
				return 1;
	
			}
		if(kernelEventData.key.raw == 146) //esc
			{
			//opl3_quietAll();
			return -1;
			}
		if(kernelEventData.key.raw == 0x20) //space for pause
			{
			return -2;
			}
		if(kernelEventData.key.raw == 0x87) //F7
			{
			return -3;
			}
		if(kernelEventData.key.raw == 0x93) //tab
			{
			return -4;
			}
	}
return 0;
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
	POKE(VKY_GR_CLUT_0+c, FAR_PEEK(0x10000+c)); //palette for piano
}
POKE(MMU_IO_CTRL,0); //MMU I/O to page 0
	
bitmapSetAddress(0,0x10400);
bitmapSetVisible(0,false);
initFPR();
//resetGlobals(gPtr);

POKE(MMU_IO_CTRL,0); //MMU I/O to page 0

//resetMID(&gMID);
}


void snoopLoop() //reached when in paused mode
	{
	uint8_t escape = 1;

	bitmapSetVisible(0,true);
	pauseTextUI();
	textSetColor(15,1);
	
	while(escape)
		{
		//dealMIDIIn(&gMID);
		kernelNextEvent();
	
		if(kernelEventData.type == kernelEvent(key.PRESSED))
			{
			if(kernelEventData.key.raw == 0x20) escape = 0; //spacebar
			}
	
		}
	
	//restoration of the status of the chip before pausing
	textUI();
	}


uint8_t mainLoop()
{
bool iRT = false;
bool pauseRequested = false;
int checkKey = 0;
uint8_t exitFlag = 1;

	
while(true)//main play and input loop
	{
		int8_t playbackReturn = playbackMOD(theFile, iRT, pauseRequested);
		//textGotoXY(60,10);printf("pause %d",pauseRequested);
		if(playbackReturn == -1) //done with file
			{
			pauseRequested = false;
			break;
			}
		if(playbackReturn == -2) //pause conditions met
			{
			pauseRequested = false;
			//opl3_quietAll();
			//axes_info(START_HEIGHT_VIS);
			//show_all_inst();
			//snoopLoop();
			//if(iRT==false) wipe_inst();
			}
		checkKey = dealPressedKey();
		if(checkKey == 1) //f3 load is hit
			{
			exitFlag = 2;
			break;
			}
		if(checkKey == -1) //esc is hit
			{
			exitFlag = 0;
			break;
			}
		if(checkKey == -2) //pause (space) is hit
			{
			pauseRequested = true;
			}
		if(checkKey == -3) //F7 is hit
			{
			if(iRT)
			{
				//wipe_inst();
				iRT = false;
			}
			else
				{
				//axes_info(START_HEIGHT_VIS);
				//show_all_inst();
				iRT = true;
				}
			}
		if(checkKey == -4) //tab is hit
			{
				pauseRequested = false;
				break;
			}
	}
	return exitFlag;
}
long countLines(FILE *fp) {
    long lines = 0;
    char c;

    while (fileRead(&c, 1, 1, fp) == 1) {
        if (c == '\n')
            lines++;
    }

    return lines;
}

int readLine(FILE *fp, char *buf, int max) {
    int i = 0;
    char c;

    while (fileRead(&c, 1, 1, fp) == 1) {
		if (c == 0x0a) //line feed control detected
            continue;
	
        if (c == 0x0d) //carriage return detected
            break;

        if (i < max - 1)
            buf[i++] = c;
    }

    if (i == 0 && c != 0x0d)
        return 0; // EOF

    buf[i] = '\0';
    return 1;
}

void fetchNextPLEntry(FILE *playlistFile, uint16_t *curLine, uint16_t nbLines)
{
if(playlistFile == NULL) return; //not normal to not have a playlist when using this function

for(uint8_t z=0;z<MAX_FILENAME_LEN;z++)
	{
	name[z]=0; //empty out the global extern name
	}
readLine(playlistFile, name, MAX_FILENAME_LEN);
lilpause(1);
*curLine = (*curLine)++;
if(*curLine == nbLines)
	{
	fseek(playlistFile, 0, SEEK_SET);
	*curLine=0;
	}
}

int main(int argc, char *argv[]) {
bool cliFile = false; //first time it loads, next times it keeps the cursor position
bool isPlayListing = false; //becomes true if a .spl simple playlist file has been opened and is being dealt with

uint8_t exitFlag = 1;

FILE *playlistFile;

setup();
	
if(argc == 3)
	{
	uint8_t i=0, j=0,m=0;
	char *lastSlash;
	size_t dirLen=0;
	char cliPath[MAX_PATH_LEN];
	
	
	while(argv[1][i] != '\0') //copy the directory
		{
			cliPath[i] = argv[1][i];
			i++;
		}
	cliPath[i] = '\0';
	
	while(argv[1][j] != '\0') //copy the filename
		{
	
			if(argv[1][j]=='.') m++;
			if(m==1 && (argv[1][j]=='m' || argv[1][j]=='M')) m++;
			else m=0;
			if(m==2 && (argv[1][j]=='o' || argv[1][j]=='O')) m++;
			else m=0;
			if(m==3 && (argv[1][j]=='d' || argv[1][j]=='D')) m++;
			else m=0;
			name[j] = argv[1][j];
			j++;
			if(m==4){name[j] = '\0'; break;}
		}
	lastSlash = strrchr(name, '/');
	dirLen = lastSlash - name; // include '/'
	
	memset(cliPath, 0, sizeof(cliPath));
	strncpy(cliPath, name, dirLen);
	cliPath[dirLen] = '\0';
	fpr_set_currentPath(cliPath);
	
	//sprintf(name, "%s%s%s", fpr.currentPath,"/", fpr.selectedFile);
	
	//printf("\nfinal form is: ");
	//printf("%s",name);
	
	cliFile = true;
	}

	
	

//if(hasCaseLCD())displayImage(LCDBIN, 2);
lilpause(1);


uint16_t nbLines = 538;
uint16_t curLine = 0;

while(exitFlag > 0)
	{
	bool foundOne = false;
	
	theFile = NULL;
	instructions();
	
	if(theFile!=NULL){fileClose(theFile); theFile=NULL;}
	
	while(!foundOne)
		{
		if(cliFile == false) //force a file pick if the command line arguments were not set
			{
				if(isPlayListing) //will not enter this block the first time around since a playlist has to be chosen during runtime to get to one
					{
					fetchNextPLEntry(playlistFile, &curLine, nbLines); //next .vgm gets fetched from an active opened playlist file
					}
				else
					{
						if(getTheFile_far(name)!=0) return 0; //calls the file picker and force the user to pick something, either .vgm or .spl
					}
				eraseLine(3);eraseLine(4);
				textGotoXY(0,3);textSetColor(7,0);printf("Loading: %.71s",name);
	
				//at this point, must determine if it's a .VGM or a .SPL
				const char *extension = name + strlen(name) - 3; //gets last 3 characters
	
				if(strcmp(extension,"spl") == 0) //it's a playlist that has been picked for the first time, feed a .vgm into the name string
					{
					isPlayListing = true;
					playlistFile = fileOpen(name,"r");
	
					fetchNextPLEntry(playlistFile, &curLine, nbLines); //next .vgm gets fetched from an active opened playlist file
					}
				theFile = load_MOD_file(name); //reaching this point, name WILL carry a .vgm file name and it must be opened.
				lilpause(1);
			}
		else theFile = load_MOD_file(name); //if there was a CL argument, then use the fetched name to load the vgm directly.

		if(theFile != NULL) foundOne = true;
		else { //not supposed to happen
			textGotoXY(0,2);printf("isNULL");
			hitspace();
			}
		} //end of while(!foundOne)
	
	checkMODHeader(theFile);
	eraseLine(3);eraseLine(4);
	eraseLine(5);eraseLine(6);
	eraseLine(7);eraseLine(8);
	textGotoXY(0,3);textSetColor(7,0);printf("Playing: %.71s",name);
	cliFile = false; //from the 2nd file and on, the cli argument is no longer used
	//info on screen
	textUI();
	comeRightTrough = true; //to kickstart it
	
	exitFlag = mainLoop();
	if(exitFlag == 2 && isPlayListing)
		{
		fileClose(playlistFile);
		playlistFile = NULL;
		isPlayListing = false;
		}
	}


if(isPlayListing) {fileClose(playlistFile);}

//opl3_initialize();
return 0;}

