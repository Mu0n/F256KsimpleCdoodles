#include "D:\F256\llvm-mos\code\mandel\.builddir\trampoline.h"

#define F256LIB_IMPLEMENTATION

#define PAL_BASE    0x10000

#include "../src/muUtils.h" //contains helper functions I often use

EMBED(palback, "../assets/mandel.pal", 0x10000); //1kb

void dopixel(uint16_t, uint16_t, uint8_t);
void setup();


void setup()
{
	uint16_t c;
	
	POKE(MMU_IO_CTRL, 0x00);
	// XXX GAMMA  SPRITE   TILE  | BITMAP  GRAPH  OVRLY  TEXT
	POKE(VKY_MSTR_CTRL_0, 0b00001111); //sprite,graph,overlay,text
	// XXX XXX  FON_SET FON_OVLY | MON_SLP DBL_Y  DBL_X  CLK_70
	POKE(VKY_MSTR_CTRL_1, 0b00010100); //font overlay, double height text, 320x240 at 60 Hz;
	POKE(VKY_LAYER_CTRL_0, 0b00010000); //bitmap 0 in layer 0, bitmap 1 in layer 1
	POKE(VKY_LAYER_CTRL_1, 0b00000010); //bitmap 2 in layer 2
	POKE(0xD00D,0x00); //force black graphics background
	POKE(0xD00E,0x00);
	POKE(0xD00F,0x00);

	POKE(MMU_IO_CTRL,1);  //MMU I/O to page 1
	// Set up CLUT0.
	for(c=0;c<1023;c++)
	{
		POKE(VKY_GR_CLUT_0+c, FAR_PEEK(PAL_BASE+c));
	}

	POKE(MMU_IO_CTRL, 0x00);
	
	bitmapSetActive(0);
	bitmapSetCLUT(0);
	
	bitmapSetVisible(0,true);
	bitmapSetVisible(1,false);
	bitmapSetVisible(2,false);

}

void dopixel(uint16_t x,uint16_t y,uint8_t c)
{
	bitmapSetColor(c);
	bitmapPutPixel(x,y);
}
int main(int argc, char *argv[]) {
	uint16_t x,y;
	float x0,y0,xf,yf,x2,y2,result;
	//uint32_t x0,y0,xf,yf,x2,y2,result;
	uint8_t iter;
	
	setup();
	
	for(y=0;y<240;y++)
	{
		for(x=0;x<320;x++)
		{/*
			result = 0;
			iter=0;xf=0;yf=0;x2=0;y2=0;
			x0=-6400 + 30*x;
			y0=-3200 + 27*y;
			while(iter < 40)
			{
				x2 = xf*xf; y2= yf*yf;
				result = x2 + y2;
				if(result > 12800) break;
				xf = x2 - y2 + x0;
				yf = 2*xf*yf+ y0;
				iter++;
			}*/
	
			result = 0.0f;
			iter=0.0f;xf=0.0f;yf=0.0f;x2=0.0f;y2=0.0f;
			x0=-2.0f + (float)x/106.666f;
			y0=-1.0f + (float)y/120.0f;
			while(iter < 20)
			{
				x2 = xf*xf; y2= yf*yf;
				result = x2 + y2;
				if(result > 4.0f) break;
				xf = x2 - y2 + x0;
				yf = 2.0f*xf*yf+ y0;
				iter++;
			}
	
			dopixel(x,y,0xff-iter);
	
		}
	}
	while(true);
	return 0;
}
