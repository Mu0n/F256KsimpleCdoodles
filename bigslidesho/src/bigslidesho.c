#define F256LIB_IMPLEMENTATION
#include "f256lib.h"

#include "../src/muUtils.h" //contains helper functions I often use
#include <stdlib.h>

#define TIMER_FRAMES 0
#define TIMER_SECONDS 1


// Global playback variables


EMBED(earthgfx, "../assets/earth.bin", 0x200000); //64000 bytes, end at 0x1FA00
EMBED(earthpal, "../assets/earth.pal", 0x1FA00);

void eraseLine(uint8_t line)
{	
textGotoXY(0,line);printf("                                                                                ");
}

void setup()
{
POKE(MMU_IO_CTRL, 0x00);
// XXX GAMMA  SPRITE   TILE  | BITMAP  GRAPH  OVRLY  TEXT
POKE(VKY_MSTR_CTRL_0, 0b00001111); //sprite,graph,overlay,text
// XXX XXX  FON_SET FON_OVLY | MON_SLP DBL_Y  DBL_X  CLK_70
POKE(VKY_MSTR_CTRL_1, 0b00010101); //font overlay, double height text, 320x240 at 60 Hz;


POKE(0xD00D,0x00); //force black graphics background
POKE(0xD00E,0x00);
POKE(0xD00F,0x00);
}

void loadPAL(uint32_t addr, uint8_t clut)
{
	POKE(MMU_IO_CTRL,1);  //MMU I/O to page 1
	for(int16_t c=0;c<1023;c++) 
	{
		POKE(VKY_GR_CLUT_0+c, FAR_PEEK(addr+c));
	}
	POKE(MMU_IO_CTRL,0);
}

void loadGFX()
{
	graphicsSetLayerBitmap(0,0);
	loadPAL(0x1FA00, 0);
	bitmapSetAddress(0, 0x200000);
	bitmapSetActive(0);
	bitmapSetCLUT(0);
	bitmapSetVisible(0, true);
}
void activate2X()
{
	uint8_t mmumemctrl = PEEK(MMU_MEM_CTRL);
	uint8_t mmuiomctrl = PEEK(MMU_IO_CTRL);
	POKE(MMU_MEM_CTRL, mmumemctrl | 0b00000100); //SRAM enable
	POKE(MMU_IO_CTRL, mmuiomctrl | 0b00011100); //move flash, move io, access io ext
}
int main(int argc, char *argv[]) {
uint8_t exitFlag = 0;


setup();
loadGFX();
activate2X();

printf("allo");

while(exitFlag == 0)
	{
	kernelNextEvent();
	if(kernelEventData.type == kernelEvent(key.PRESSED))
		{
		if(kernelEventData.key.raw == 146) //esc
			{
			return 0;
			}
		}
	}

return 0;}

}