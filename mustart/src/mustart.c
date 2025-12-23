#define F256LIB_IMPLEMENTATION
#include "f256lib.h"

#include "../src/muUtils.h" //contains helper functions I often use
#include "../src/muMenu.h" //contains helper functions I often use

#include <stdio.h>
#include <stdlib.h>

FILE *theFile;

//prototypes:
void setup(void);
int8_t dealPressedKey(void);


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
			}
		if(kernelEventData.key.raw == 0x87) //F7
			{		
			}
		if(kernelEventData.key.raw == 0x93) //tab
			{		
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

POKE(MMU_IO_CTRL,0); //MMU I/O to page 0
}


int main(int argc, char *argv[]) {

uint8_t exitFlag = 0;

setup();

displayMenu(5,10);
hitspace();


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

kernelArgs->common.buf = (char *)0x0200;
kernelArgs->common.buflen = 2;
kernelArgs->common.ext = (char *)0x0280;
kernelArgs->common.extlen = 6;

uint8_t pathLen = strlen("vgmplayer2.pgz") + 1;
strcpy((char *)0x0202, "vgmplayer2.pgz");

//arg0
*(uint8_t*)0x0200 = '-';
*(uint8_t*)0x0201 = 0;
*(uint8_t*)0x0280 = 0x00;
*(uint8_t*)0x0281 = 0x02;
  
//arg1
*(uint8_t*)0x0282 = 0x02;
*(uint8_t*)0x0283 = 0x02;
*(uint8_t*)0x0284 = 0x00;  

kernelCall(RunNamed);


return 0;}

}