#include "D:\F256\llvm-mos\code\hackGfx\.builddir\trampoline.h"

#define F256LIB_IMPLEMENTATION
#include "f256lib.h"
#include "../src/muUtils.h" //contains helper functions I often use
#include "../src/muopl3.h" //opl3 chip routines
#include "../src/muvgmplay.h" //used to play vgms
#include "../src/muTimer0Int.h"  //timer0 routines with an interval of max 2/3 of a second

#define PAL_BASE    0x10000
#define TIL_L2_MAP  0x20000 //40*17*2 = 1360 bytes
#define TIL_SET     0x4B400
#define TIL_L3_MAP  0x4BF00 //40*17*2 = 1360 bytes

#define SCROLL_COOKIE 0
#define SCROLL_DELAY 2

#define TIMER_FRAMES 0
#define TIMER_SECONDS 1

#define KINDS_OF_TILES 20
#define GSHIFT 16
#define END_X_SCROLL 39
#define END_Y_SCROLL 30

EMBED(pal,    "../assets/gfxHack.pal", 0x10000); //1024 b
EMBED(tile3, "../assets/gfxHack.bin", 0x4B400);// 1024 b

uint16_t tm_xf = 0; //fine scroll for tile map 2
uint8_t tm_x = 0; //crude scroll for tile map 2
uint16_t tm_xf1 = 0;
uint8_t tm_x1 = 0;
bool wantText = true;
bool wantSound = true;

FILE *theFile;

struct timer_t tileAnim;

void toggleWantText()
{
if(wantText)
{
	wantText = false;
// DIS GAMMA  SPRITE   TILE  | BITMAP  GRAPH  OVRLY  TEXT
POKE(VKY_MSTR_CTRL_0, 0b01111110); //sprite,graph,overlay,text
}
else
{
	wantText = true;
// DIS GAMMA  SPRITE   TILE  | BITMAP  GRAPH  OVRLY  TEXT
POKE(VKY_MSTR_CTRL_0, 0b01111111); //sprite,graph,overlay,text
}
}

uint8_t dealKey(uint8_t raw)
{
	switch(raw)
	{
		case 0x92: //ESC
			return 0xFF;
		case 0x20: //space
			toggleWantText();
			break;
		case 0x73: //S
			if(wantSound)
			{
				opl3_quietAll();
				wantSound = false;
			}
			else wantSound = true;
			break;
	}
	return 0;
}

void dealTimer(uint8_t cookie)
{
	switch(cookie)
	{
		case SCROLL_COOKIE:
			tm_xf++;
			if(tm_xf > 8)
				{
					tm_xf=0;
	
					tm_x++;
					if(tm_x > END_X_SCROLL) tm_x = 0;
				}
			tileSetScroll(2, tm_xf, tm_x, 0, 0);

			if(tm_xf1 == 0)
			{
				tm_xf1=8;
				if(tm_x1 == 0) tm_x1 = END_X_SCROLL;
				else tm_x1--;
			}
			else tm_xf1--;
			tileSetScroll(1, tm_xf1, tm_x1, 0, 0);
	
	
			tileAnim.absolute = getTimerAbsolute(TIMER_FRAMES) + SCROLL_DELAY;
			setTimer(&tileAnim);
			break;
	}
}
void setup()
{
//sets the gfx vicky mode
POKE(MMU_IO_CTRL, 0x00);
// DIS GAMMA  SPRITE   TILE  | BITMAP  GRAPH  OVRLY  TEXT
POKE(VKY_MSTR_CTRL_0, 0b01111111); //sprite,graph,overlay,text
// MT_Bk MT_En FON_SET FON_OVLY | MON_SLP DBL_Y  DBL_X  CLK_70
POKE(VKY_MSTR_CTRL_1, 0b00010000); //font overlay, 320x240 at 60 Hz;

POKE(VKY_LAYER_CTRL_0, 0b01010000); //bitmap 0 in layer 0, tile 1 in layer 1
POKE(VKY_LAYER_CTRL_1, 0b00000110); //tile 2 in layer 2


POKE(MMU_IO_CTRL,1);  //MMU I/O to page 1
	// Set up CLUT0.
for(uint16_t c=0;c<1023;c++)  //copy clut0 for regular graphics
	{
		POKE(VKY_GR_CLUT_0+c, FAR_PEEK(PAL_BASE+c));
	}
POKE(MMU_IO_CTRL, 0);

//background color layer
POKE(0xD00D,0x00); //force black graphics background
POKE(0xD00E,0x00);
POKE(0xD00F,0x00);


//tiles

tileDefineTileMap(1, TIL_L2_MAP, 8, 80, 31); //40x15 map of 16x16, tile map2, making sure it's twice as large as the screen. B[AB]A data repetition for endless scroll
tileDefineTileMap(2, TIL_L3_MAP, 8, 80, 31); //40x15 map of 16x16, tile map2, making sure it's twice as large as the screen. B[AB]A data repetition for endless scroll
tileDefineTileSet(0, TIL_SET, false); //in a strip graphic data, tile set0

for(uint16_t y=0;y<31;y+=2) //straight blue sky
{
	for(uint16_t x=0;x<40;x++)
	{
		FAR_POKEW(TIL_L3_MAP + (uint32_t)(x*2) + (uint32_t)(y*80*2), 0x0000 | randomRead()%KINDS_OF_TILES); //clut 0, set 2, tile number 0
		FAR_POKEW(TIL_L3_MAP + (uint32_t)(x*2) + (uint32_t)((y+1)*80*2), 0x0000 | 0); //clut 0, set 2, tile number 0
	}
}
for(uint16_t y=0;y<31;y+=2) //straight blue sky
{
	for(uint16_t x=40;x<80;x++)
	{
		FAR_POKEW(TIL_L3_MAP + (uint32_t)(x*2) + (uint32_t)(y*80*2), FAR_PEEK(TIL_L3_MAP + (uint32_t)((x-40)*2) + (uint32_t)(y*80*2))); //clut 0, set 2, tile number 0
		FAR_POKEW(TIL_L3_MAP + (uint32_t)(x*2) + (uint32_t)((y+1)*80*2), 0); //clut 0, set 2, tile number 0
	}
}


for(uint16_t y=0;y<31;y+=2) //straight blue sky
{
	for(uint16_t x=0;x<40;x++)
	{
		FAR_POKEW(TIL_L2_MAP + (uint32_t)(x*2) + (uint32_t)((y)*80*2), 0x0000 | 0); //clut 0, set 2, tile number 0
		FAR_POKEW(TIL_L2_MAP + (uint32_t)(x*2) + (uint32_t)((y+1)*80*2), 0x0000 | randomRead()%KINDS_OF_TILES); //clut 0, set 2, tile number 0
	}
}
for(uint16_t y=0;y<31;y+=2) //straight blue sky
{
	for(uint16_t x=40;x<80;x++)
	{
		FAR_POKEW(TIL_L2_MAP + (uint32_t)(x*2) + (uint32_t)((y)*80*2), 0x0000 | 0); //clut 0, set 2, tile number 0
		FAR_POKEW(TIL_L2_MAP + (uint32_t)(x*2) + (uint32_t)((y+1)*80*2), FAR_PEEK(TIL_L2_MAP + (uint32_t)((x-40)*2) + (uint32_t)((y+1)*80*2))); //clut 0, set 2, tile number 0
	}
}

tileSetScroll(1, 0, 20, 0, 0);
tileSetScroll(2, 0, 0, 0, 0);
tileSetVisible(2,true);
tileSetVisible(1,true);


POKE(MMU_IO_CTRL,0x00); //MMU I/O to page 0

opl3_initialize();
theFile = load_VGM_file("media/vgm/sshockt.vgm");
checkVGMHeader(theFile);

textGotoXY(10,10);textPrint("Make yourself comfortable, hacker. Stay a while.");
textGotoXY(20,12);textPrint("-Shodan");


tileAnim.units = TIMER_FRAMES;
tileAnim.cookie = SCROLL_COOKIE;
tileAnim.absolute = getTimerAbsolute(TIMER_FRAMES) + SCROLL_DELAY;
setTimer(&tileAnim);

}


int main(int argc, char *argv[]) {
int8_t playbackReturn ;

setup();

	
while(true)//main play and input loop
	{
		if(wantSound) playback(theFile, true, false);
		kernelNextEvent();
		if(kernelEventData.type == kernelEvent(timer.EXPIRED))
			{
			dealTimer(kernelEventData.timer.cookie);
			}
	
		if(kernelEventData.type == kernelEvent(key.PRESSED))
			{
			if(dealKey(kernelEventData.key.raw) == 0xFF) return 0;
			}

	}

	return 0;
} //extra brace, a bug of llvm-mos
