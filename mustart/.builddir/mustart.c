#include "D:\F256\llvm-mos\code\mustart\.builddir\trampoline.h"

#define F256LIB_IMPLEMENTATION
#include "f256lib.h"

#include "../src/muUtils.h" //contains helper functions I often use
#include "../src/muMenu.h" //contains helper functions I often use
#include "../src/menuSound.h" //contains helper functions I often use
#include "../src/mupads.h"

#include <stdio.h>
#include <stdlib.h>

EMBED(pal, "../assets/bm.pal", 0x10000);
EMBED(bmmenu, "../assets/bm.bin", 0x10400);

FILE *theFile;

//prototypes:
void setup(void);
int8_t dealPressedKey(void);


uint8_t selected = 0;
uint8_t catSelected = 0;

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

int8_t dealPressedKey()
{
kernelNextEvent();
if(kernelEventData.type == kernelEvent(key.PRESSED))
	{
		if(kernelEventData.key.raw == 0x83) //F3
			{
			}
		if(kernelEventData.key.raw == 146) //esc
			{
			return -1;
			}
		if(kernelEventData.key.raw == 0x20) //space for pause
			{
			killSound();
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
if(kernelEventData.type == kernelEvent(timer.EXPIRED))
	{
		if(kernelEventData.timer.cookie == TIMER_MENUSOUND_COOKIE)
		{
			psgNoteOff(0, PSG_BOTH);
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
setStereoPSG();
}


int main(int argc, char *argv[]) {
uint8_t exitFlag = 0;

setup();

readMenuContent();

displayMenu(MENUTOPX,MENUTOPY,0);
textSetColor(13,1);
displayOneItem(MENUTOPX, MENUTOPY, 0,0);

while(!exitFlag)
{
	int8_t whichKey = 0;
	whichKey = dealPressedKey();
	switch(whichKey)
	{
		case -1:
			kernelArgs->common.buf = "basic";
			kernelArgs->common.buflen = 11;
			gfxCleanUp();
			kernelCall(RunNamed);
			break;
		case 1:
			exitFlag = 1;
			break;
		case 2:
	
			kernelArgs->common.buf = "fm";
			kernelArgs->common.buflen = 3;

			gfxCleanUp();
			kernelCall(RunNamed);
			break;
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

uint8_t pathLen = strlen(cats[catSelected].items[selected].file) + 1;



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
kernelCall(RunNamed);


return 0;}

