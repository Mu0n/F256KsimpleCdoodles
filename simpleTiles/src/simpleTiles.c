#define F256LIB_IMPLEMENTATION
#define TIMER_SCROLL_DELAY 3
#define TIMER_SCROLL_COOKIE 0
#define TIMER_FRAMES 0
#include "f256lib.h"

/*

//;
//; MMU Registers
//;

#define MMU_MEM_CTRL   0x0000            // MMU Memory Control Register
#define MMU_IO_CTRL   0x0001             // MMU I/O Control Register

BM and Tile Map Layer registers p.29
#define VKY_LAYER_CTRL_0   0xD002        // Vicky Layer Control Register 0
#define VKY_LAYER_CTRL_1   0xD003        // Vicky Layer Control Register 1

i/o page 1 p.26
#define VKY_GR_CLUT_0   0xD000           // Graphics LUT #0
#define VKY_GR_CLUT_1   0xD400           // Graphics LUT #1
#define VKY_GR_CLUT_2   0xD800           // Graphics LUT #2
#define VKY_GR_CLUT_3   0xDC00           // Graphics LUT #3

*/

//map: midi, midi1, midi2, midi3
//sprite: midilogo (2 layers, last = midi, 2ndtolast = psg
//palette: bachbm.pal
//palette tilesetBach.pal
//tile file tilesetBach.tile

EMBED(xaa, "../assets/xaa", 0x10000);
EMBED(xab, "../assets/xab", 0x18000);
EMBED(xac, "../assets/xac", 0x20000);
EMBED(palbach, "../assets/bachbm2.pal", 0x28000);
EMBED(paltiles, "../assets/tileset.pal", 0x30000);
EMBED(tileset1, "../assets/tileset1.tile",0x38000);
EMBED(tileset2, "../assets/tileset2.tile",0x40000);
EMBED(midimap, "../assets/midimap1.map",0x48000);

struct timer_t scrollTimer;
int16_t SIN[] = {
       0,    1,    3,    4,    6,    7,    9,   10,
      12,   14,   15,   17,   18,   20,   21,   23,
      24,   25,   27,   28,   30,   31,   32,   34,
      35,   36,   38,   39,   40,   41,   42,   44,
      45,   46,   47,   48,   49,   50,   51,   52,
      53,   54,   54,   55,   56,   57,   57,   58,
      59,   59,   60,   60,   61,   61,   62,   62,
      62,   63,   63,   63,   63,   63,   63,   63,
      64,   63,   63,   63,   63,   63,   63,   63,
      62,   62,   62,   61,   61,   60,   60,   59,
      59,   58,   57,   57,   56,   55,   54,   54,
      53,   52,   51,   50,   49,   48,   47,   46,
      45,   44,   42,   41,   40,   39,   38,   36,
      35,   34,   32,   31,   30,   28,   27,   25,
      24,   23,   21,   20,   18,   17,   15,   14,
      12,   10,    9,    7,    6,    4,    3,    1,
       0,   -1,   -3,   -4,   -6,   -7,   -9,  -10,
     -12,  -14,  -15,  -17,  -18,  -20,  -21,  -23,
     -24,  -25,  -27,  -28,  -30,  -31,  -32,  -34,
     -35,  -36,  -38,  -39,  -40,  -41,  -42,  -44,
     -45,  -46,  -47,  -48,  -49,  -50,  -51,  -52,
     -53,  -54,  -54,  -55,  -56,  -57,  -57,  -58,
     -59,  -59,  -60,  -60,  -61,  -61,  -62,  -62,
     -62,  -63,  -63,  -63,  -63,  -63,  -63,  -63,
     -64,  -63,  -63,  -63,  -63,  -63,  -63,  -63,
     -62,  -62,  -62,  -61,  -61,  -60,  -60,  -59,
     -59,  -58,  -57,  -57,  -56,  -55,  -54,  -54,
     -53,  -52,  -51,  -50,  -49,  -48,  -47,  -46,
     -45,  -44,  -42,  -41,  -40,  -39,  -38,  -36,
     -35,  -34,  -32,  -31,  -30,  -28,  -27,  -25,
     -24,  -23,  -21,  -20,  -18,  -17,  -15,  -14,
     -12,  -10,   -9,   -7,   -6,   -4,   -3,   -1,
};


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

void setup()
{
	uint16_t c; //loop variable
	
	POKE(MMU_IO_CTRL, 0x00);
	POKE(VKY_MSTR_CTRL_0, 0x3F); //graphics and tile enabled
	POKE(VKY_MSTR_CTRL_1, 0); //320x240 at 60 Hz
	POKE(VKY_LAYER_CTRL_0, 0x40); //tile 0 in layer 1, bm 0 in layer 0
	
	
	POKE(MMU_IO_CTRL, 0x01); //set to i/o page 1 
	
	//copy over palette data to CLUT1
	for(c=0;c<1023;c++) {
		POKE(VKY_GR_CLUT_0+c, FAR_PEEK(0x28000+c));
		POKE(VKY_GR_CLUT_1+c, FAR_PEEK(0x30000+c));
	}
	POKE(MMU_IO_CTRL,0); //go back to i/o page 0
	
	//enable a bitmap
	bitmapSetAddress(0,0x10000);
	bitmapSetActive(0);
	bitmapSetVisible(0,true);
	bitmapSetVisible(1,false);
	bitmapSetVisible(2,false);
	bitmapSetCLUT(0);

	bitmapSetActive(1);
	
	//tiles setting
	tileDefineTileMap(0,0x48000,16,23,18);
	tileDefineTileSet(0,0x38000, false);
	tileSetScroll(1, 0, 0, 0, 0);
	
	tileSetVisible(0,true);
	tileSetVisible(1,false);
	tileSetVisible(2,false);
	
	scrollTimer.units = TIMER_FRAMES;
	scrollTimer.absolute = TIMER_SCROLL_DELAY;
	scrollTimer.cookie = TIMER_SCROLL_COOKIE;
}
int main(int argc, char *argv[]) {
	uint8_t offX=0, offY=128;
	uint8_t sinx=0, siny=0;
	
	setup();
	scrollTimer.absolute = getTimerAbsolute(TIMER_FRAMES) + TIMER_SCROLL_DELAY;
	setTimer(&scrollTimer);
	while(true)
		{
		kernelNextEvent();
        if(kernelEventData.type == kernelEvent(timer.EXPIRED))
            {
			switch(kernelEventData.timer.cookie)
				{
				case TIMER_SCROLL_COOKIE:
					offX++; offY+=2;
					sinx = SIN[offX]>>1; siny = SIN[offY]>>1;
					tileSetScroll(0, (sinx)%32,0, (siny)%32,0);
					scrollTimer.absolute = getTimerAbsolute(TIMER_FRAMES) + TIMER_SCROLL_DELAY;
					setTimer(&scrollTimer); 
					break;
				}
			}
		}
	return 0;}
