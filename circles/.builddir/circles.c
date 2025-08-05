#include "D:\F256\llvm-mos\code\circles\.builddir\trampoline.h"

#define F256LIB_IMPLEMENTATION
#include "f256lib.h"
#define WIDTH 320
#define HEIGHT 240

#include "../src/muUtils.h" //contains helper functions I often use


void drawFilledCircle(uint16_t x0, uint16_t y0, uint16_t radius, uint8_t col) {
	
bitmapSetColor(col);
    for (int16_t y = -radius; y <= (int16_t)radius; y++) {
        int16_t yy = (int16_t)y0 + y;
		//int16_t ys = y * y;
	
        if (yy < 0 || yy >= HEIGHT) continue;
	
		bitmapLine(x0-radius, yy, x0+radius, yy);
/*
        for (int16_t x = -radius; x <= (int16_t)radius; x++) {
            int16_t xx = (int16_t)x0 + x;
            if (xx < 0 || xx >= WIDTH) continue;

            if ((uint32_t)(x * x + ys) <= (uint32_t)(radius * radius)) {
 
				FAR_POKE(0x6c000 + xx + WIDTH*yy, col);
				//bitmapPutPixel((uint16_t)xx, (uint16_t)yy);
            }
        }
		*/
    }
}

int main(int argc, char *argv[]) {
uint16_t x,y,r,c;

POKE(MMU_IO_CTRL, 0x00);
// XXX GAMMA  SPRITE   TILE  | BITMAP  GRAPH  OVRLY  TEXT
POKE(VKY_MSTR_CTRL_0, 0b00001111); //sprite,graph,overlay,text
// XXX XXX  FON_SET FON_OVLY | MON_SLP DBL_Y  DBL_X  CLK_70
POKE(VKY_MSTR_CTRL_1, 0b00010000); //font overlay, double height text, 320x240 at 60 Hz;
POKE(VKY_LAYER_CTRL_0, 0b00010000); //bitmap 0 in layer 0, bitmap 1 in layer 1
POKE(0xD00D,0x00); //force black graphics background
POKE(0xD00E,0x00);
POKE(0xD00F,0x00);

bitmapSetActive(0);
bitmapSetCLUT(0);
bitmapSetColor(4);
bitmapSetVisible(0,true);
bitmapSetVisible(1,false);
bitmapSetVisible(2,false);



bitmapSetColor(0);
for(uint16_t xi = 0; xi<320; xi++)
{
	for(uint8_t yi=0; yi<240; yi++)
	{
	bitmapPutPixel(xi, yi);
	}
}
printf("go");
while(true)
{
	x = randomRead();
	r = randomRead();
	
//bitmapSetColor((uint8_t)(c&0x00FF));
drawFilledCircle(x>>8, x&0x00FF, r>>10, (uint8_t)(r&0x00FF));
}

return 0;}

