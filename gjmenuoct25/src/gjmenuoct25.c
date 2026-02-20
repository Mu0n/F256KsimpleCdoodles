#define F256LIB_IMPLEMENTATION
#include "f256lib.h"

#include "../src/muUtils.h" //contains helper functions I often use
#include "../src/muMenu.h" //contains helper functions I often use
#include "../src/menuSound.h" //contains helper functions I often use
#include "../src/mupads.h"
#include "../src/muMidiPlay2.h"
#include "../src/muTimer0Int.h"

#include <stdio.h>
#include <stdlib.h>

#define PAD_DELAY_COOKIE 3
#define PAD_TIMER_DELAY 12
#define MUSIC_BASE 0x30000

EMBED(pal, "../assets/bm.pal", 0x10000);
EMBED(bmmenu, "../assets/GameJamOct2025Menu.bin", 0x10400);
EMBED(canyon, "../assets/onestop.mid", 0x30000);

FILE *theFile;

//prototypes:
void setup(void);
int8_t dealPressedKey(void);


uint8_t selected = 0;
uint8_t catSelected = 0;
struct timer_t padDelayTimer;
uint8_t readyForPads = 0;
uint8_t isJingleActive = 0; //allows the quick test for sound

void quickMIDSet(void);
void quickPSGSet(void);

/*
void defInstruments()
{
	copySidInstrument(sid_instrument_defs[2], &sidInstruments[0]); //sawtooth
	sid_setInstrument(0, 0, sidInstruments[0]);
	
	copySidInstrument(sid_instrument_defs[0], &sidInstruments[1]); //sinus
	sid_setInstrument(0, 1, sidInstruments[1]);
	
	copySidInstrument(sid_instrument_defs[3], &sidInstruments[2]); //noise
	sid_setInstrument(0, 2, sidInstruments[2]);
	
	copySidInstrument(sid_instrument_defs[1], &sidInstruments[3]); //triangle
	sid_setInstrument(1, 0, sidInstruments[3]);
	
	copySidInstrument(sid_instrument_defs[1], &sidInstruments[4]); //triangle
	sid_setInstrument(1, 1, sidInstruments[4]);
	
	copySidInstrument(sid_instrument_defs[0], &sidInstruments[5]); //sinus
	sid_setInstrument(1, 2, sidInstruments[5]);
	
	sid_setSIDWide(0);
	chipXChannel[0] = 0x01; //voice 0 | sid  (sawtooth)
	chipXChannel[1] = 0x11; //voice 1 | sid  (sine)
	chipXChannel[2] = 0x31; //voice 1 | sid  (tri)
	chipXChannel[3] = 0x41; //voice 1 | sid  (tri)
	chipXChannel[4] = 0x51; //voice 1 | sid  (squ)
	chipXChannel[9] = 0x21; //voice 2 | sid  (noise)
	
}
*/
void gfxCleanUp()
{
textClear();
POKE(VKY_MSTR_CTRL_0, 0b00001111); //sprite,graph,overlay,text
POKE(VKY_MSTR_CTRL_1, 0b00000000); //font overlay, double height text, 320x240 at 60 Hz;
for(uint8_t i=0;i<64;i++)spriteSetVisible(i,false);
bitmapSetVisible(0,false);
bitmapSetVisible(1,false);
bitmapSetVisible(2,false);

textEnableBackgroundColors(false);
}

void resetPadReadiness()
{
	readyForPads = 0;
	padDelayTimer.absolute = getTimerAbsolute(TIMER_FRAMES)+PAD_TIMER_DELAY;
	setTimer(&padDelayTimer);
}
int8_t dealPressedKey()
{
	if(readyForPads==1)
	{
		pollNES();
		if(PEEK(PAD0)==0xF7 || PEEK(PAD1)==0xF7 || PEEK(PAD2)==0xF7 || PEEK(PAD3)==0xF7)
		{
			goUpOrDown(true, &selected, catSelected);
			resetPadReadiness();
		}
		else if(PEEK(PAD0)==0xFB || PEEK(PAD1)==0xFB || PEEK(PAD2)==0xFB || PEEK(PAD3)==0xFB)
		{
			goUpOrDown(false, &selected, catSelected);
			resetPadReadiness();
		}
		else if(PEEK(PAD0)==0xFD || PEEK(PAD1)==0xFD || PEEK(PAD2)==0xFD || PEEK(PAD3)==0xFD)
		{
			goLeftOrRight(true, &selected, &catSelected);
			resetPadReadiness();
		}
		else if(PEEK(PAD0)==0xFE || PEEK(PAD1)==0xFE || PEEK(PAD2)==0xFE || PEEK(PAD3)==0xFE)
		{
			goLeftOrRight(false, &selected, &catSelected);
			resetPadReadiness();
		}
		else if(PEEK(PAD0)==0x7F || PEEK(PAD1)==0x7F || PEEK(PAD2)==0x7F || PEEK(PAD3)==0x7F)
		{		
				return 1;
		}
		
		pollSNES();
		if(PEEK(PAD0)==0xF7 || PEEK(PAD1)==0xF7 || PEEK(PAD2)==0xF7 || PEEK(PAD3)==0xF7)
		{
			goUpOrDown(true, &selected, catSelected);
			resetPadReadiness();
		}
		else if(PEEK(PAD0)==0xFB || PEEK(PAD1)==0xFB || PEEK(PAD2)==0xFB || PEEK(PAD3)==0xFB)
		{
			goUpOrDown(false, &selected, catSelected);
			resetPadReadiness();
		}
		else if(PEEK(PAD0)==0xFD || PEEK(PAD1)==0xFD || PEEK(PAD2)==0xFD || PEEK(PAD3)==0xFD)
		{
			goLeftOrRight(true, &selected, &catSelected);
			resetPadReadiness();
		}
		else if(PEEK(PAD0)==0xFE || PEEK(PAD1)==0xFE || PEEK(PAD2)==0xFE || PEEK(PAD3)==0xFE)
		{
			goLeftOrRight(false, &selected, &catSelected);
			resetPadReadiness();
		}
		else if(PEEK(PAD0_S)==0xF7 || PEEK(PAD1_S)==0xF7 || PEEK(PAD2_S)==0xF7 || PEEK(PAD3_S)==0xF7)
		{		
				return 1;
		}
	}
	
	
kernelNextEvent();
if(kernelEventData.type == kernelEvent(key.PRESSED))
	{		
		if(kernelEventData.key.raw == 0x83) //F3
			{
				killSound();
					destroyTrack();
					initTrack(MUSIC_BASE);
					isJingleActive = false;
					chipActive = 0;
			}
		if(kernelEventData.key.raw == 146) //esc
			{
			return -1;
			}
		if(kernelEventData.key.raw == 0x20) //space for pause
			{
			killSound();
			if(isJingleActive) isJingleActive = false;
			else isJingleActive = true;
			}
		if(kernelEventData.key.raw == 0x87) //F7
			{		
			}
		if(kernelEventData.key.raw == 0x93) //tab
			{		
			}
		if(kernelEventData.key.raw == 0xB7) //down
			{		
			goUpOrDown(false, &selected, catSelected);
			}
		if(kernelEventData.key.raw == 0xB6) //up
			{
			goUpOrDown(true, &selected, catSelected);
			}
		if(kernelEventData.key.raw == 0xB8) //left
			{
			goLeftOrRight(true, &selected,&catSelected);
			}
		if(kernelEventData.key.raw == 0xB9) //right
			{
			goLeftOrRight(false, &selected,&catSelected);
			}
		if(kernelEventData.key.raw == 0x94) //enter
			{		
			return 1;
			}
			
		if(kernelEventData.key.raw == 0x62) //B for superbasic
			{		
			return -1;
			}
		if(kernelEventData.key.raw == 0x66) //F for f/manager
			{		
			return 2;
			}
	}
else if(kernelEventData.type == kernelEvent(timer.EXPIRED))
	{
		switch(kernelEventData.timer.cookie)
		{
			case TIMER_MENUSOUND_COOKIE:
				psgNoteOff(0, PSG_BOTH);
				break;
			case PAD_DELAY_COOKIE:
				readyForPads=1;				
				break;
		}
	}
return 0;
}

void setup()
{
POKE(MMU_IO_CTRL, 0x00);

gfxCleanUp();


// XXX GAMMA  SPRITE   TILE  | BITMAP  GRAPH  OVRLY  TEXT
POKE(VKY_MSTR_CTRL_0, 0b00001111); //sprite,graph,overlay,text
// XXX XXX  FON_SET FON_OVLY | MON_SLP DBL_Y  DBL_X  CLK_70
POKE(VKY_MSTR_CTRL_1, 0b00000100); //font overlay, double height text, 320x240 at 60 Hz;
POKE(VKY_LAYER_CTRL_0, 0b00010000); //bitmap 0 in layer 0, bitmap 1 in layer 1
POKE(0xD00D,0x00); //force black graphics background
POKE(0xD00E,0x00);
POKE(0xD00F,0x00);	


//bitmap CLUT
POKE(MMU_IO_CTRL,1);  //MMU I/O to page 1
	// Set up CLUT0.
for(uint16_t c=0;c<1023;c++) 
	{
		POKE(VKY_GR_CLUT_0+c, FAR_PEEK(0x10000+c));
	}
POKE(MMU_IO_CTRL, 0);


bitmapSetActive(0);

bitmapSetAddress(0,0x10400);
bitmapSetCLUT(0);
bitmapSetVisible(0,true);
	
textEnableBackgroundColors(true);
textSetColor(15,0);
initItems();
initMenuSoundTimer();
killSound();
setMonoPSG();
}

void quickMIDSet()
{
	killSound();

}

int main(int argc, char *argv[]) {
uint8_t exitFlag = 0;

setup();

readMenuContent();

displayMenu(MENUTOPX,MENUTOPY,0);
textSetColor(13,1);
displayOneItem(MENUTOPX, MENUTOPY, 0,0);


resetInstruments(false); //resets all channels to piano, for sam2695
midiShutUp(false); //ends trailing previous notes if any, for sam2695
	
initTrack(MUSIC_BASE);

resetTimer0();
padDelayTimer.cookie = PAD_DELAY_COOKIE;
padDelayTimer.units = TIMER_FRAMES;
resetPadReadiness();
isJingleActive=true;
fudge = 0;
while(!exitFlag)
{
	int8_t whichKey = 0;
	if(isJingleActive)
		{	
		if(PEEK(INT_PENDING_0)&0x10) //when the timer0 delay is up, go here
			{
			POKE(INT_PENDING_0,0x10); //clear the timer0 delay
			playMidi(); //play the next chunk of the midi file, might deal with multiple 0 delay stuff
			}
		if(theOne.isWaiting == false) 
			{
			sniffNextMIDI(); //find next event to play, will cue up a timer0 delay
			}
		
		if(theOne.isMasterDone >= theOne.nbTracks) 
				{
					killSound();
					
					destroyTrack();
					initTrack(MUSIC_BASE);
					isJingleActive = false;
					chipActive = 0;
					updateSoundStatus(&catSelected);
					continue; //really quit no matter what
				}
		}

					
	whichKey = dealPressedKey();

	if(whichKey == -1) 
	{
		kernelArgs->common.buf = "basic";
		kernelArgs->common.buflen = 11;
		gfxCleanUp();
killSound();
		kernelCall(RunNamed);
	}
	if(whichKey == 1) 
	{
		exitFlag = 1;
	}
	if(whichKey == 2) 
	{
		kernelArgs->common.buf = "fm";
		kernelArgs->common.buflen = 3;

		gfxCleanUp();
killSound();
		kernelCall(RunNamed);
	}
}


/*
struct call_args {
    struct events_t events;  // The GetNextEvent dest address is globally reserved.
    union {
        struct common_t   common;
        struct fs_t       fs;
        struct file_t     file;
        struct dir_t      directory;
        struct display_t  display;
        struct net_t      net;
        struct timer_t    timer;
    };
};
*/

/*

struct common_t {
    char        dummy[8-sizeof(struct events_t)];
    const void *ext;
    uint8_t     extlen;
    const void *buf;
    uint8_t     buflen;
    void *      internal;
};
*/


kernelArgs->common.buf = (char *)0x0200; // tell Kernel which buffer to work with
kernelArgs->common.buflen = 2;
kernelArgs->common.ext = (char *)0x0280; // tell Kernel where the arg pointers start and how many there are
kernelArgs->common.extlen = 4; // 2 pointers of 2 bytes each to look at

//uint8_t pathLen = strlen(cats[catSelected].items[selected].file) + 1;



	// LOGIC:
	// kernel.args.buf needs to have name of named app to run, which in this case is '-' (pexec's real name)
	// we also need to prep a different buffer with a series of pointers (2), one of which points to a string for '-', one for the app (e.g, 'modojr.pgz', and optionally one for the file the called up app should load (e.g., 'mymodfile.mod')
	// We have from $200 to $27f to use for the paths
	//   Because we only have 128 chars for all 3 paths (126 after pexec), the max len of either path is 62 (NULL terminators eat a space)
	// The pointers to the path components start at $280.
	// we set arg0 to pexec ('-'), arg1 to the path of the app to load, and arg2, if passed, to the path of the file to load
	
strcpy((char *)0x0202, cats[catSelected].items[selected].file);
//strcpy((char *)(0x0202 + pathLen), 0x00);

//  arg0: pexec "-"
*(uint8_t*)0x0200 = '-'; // tell Kernel which buffer to work with
*(uint8_t*)0x0201 = 0;

*(uint8_t*)0x0280 = 0x00; // set pointer to arg0
*(uint8_t*)0x0281 = 0x02; // first arg (pexec '-') is at $0200
  
//arg1
*(uint8_t*)0x0282 = 0x02; // set pointer to arg1
*(uint8_t*)0x0283 = 0x02; // 2nd arg (the app path) is at $0202

*(uint8_t*)0x0284 = 0x00; // terminator (will be overwritten if there is a file to load) 


gfxCleanUp();
killSound();
kernelCall(RunNamed);


return 0;}

}