#define F256LIB_IMPLEMENTATION
#include "f256lib.h"

#include "../src/muUtils.h" //contains helper functions I often use
#include <stdlib.h>

#define TIMER_FRAMES 0
#define TIMER_SECONDS 1


// Global playback variables


EMBED(earthgfx, "../assets/earth.bin", 0x10000); //64000 bytes, end at 0x1FA00
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
POKE(VKY_MSTR_CTRL_1, 0b00010101); //font overlay, double height text, 320x240 at 70 Hz;


POKE(0xD00D,0x00); //force black graphics background
POKE(0xD00E,0x00);
POKE(0xD00F,0x00);
}

void drawNewLine(uint16_t x1, uint16_t x2, uint8_t y1, uint8_t y2, uint8_t col)
{
//VICKY MASTER CONTROL REG 2
POKE(0xD00A,0x01); //drawline enable

POKE(0xD180,0x01); //drawline enable step 2
POKEW(0xD182,0x00); //set x1 to 0
POKE(0xD181,col); //line color
POKEW(0xD182,x1); //set x1 to needed value
POKEW(0xD184,x2); //set x2 to needed value
POKE(0xD186,y1); //set y1 to needed value
POKE(0xD187,y2); //set y2 to needed value
POKE(0xD180,0x03); //keep enabled and draw it
while((PEEK(0xD180)&0x80) != 0x80)
	;
//VICKY MASTER CONTROL REG 2
POKE(0xD00A,0x00); //drawline disable
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
	//loadPAL(0x1FA00, 0);
	//bitmapSetAddress(0, 0x10000);
	bitmapSetActive(0);
	bitmapSetCLUT(0);
	bitmapSetColor(0);
	textGotoXY(0,24);
	textPrint("old Lines    ");
	bitmapClear();
	bitmapSetVisible(0, true);
}
void activate2X()
{
	uint8_t mmumemctrl = PEEK(MMU_MEM_CTRL);
	uint8_t mmuiomctrl = PEEK(MMU_IO_CTRL);
	POKE(MMU_MEM_CTRL, mmumemctrl | 0b00000100); //SRAM enable
	POKE(MMU_IO_CTRL, mmuiomctrl | 0b00000000); //move flash, move io, access io ext
}
int main(int argc, char *argv[]) {
uint8_t exitFlag = 0;
uint16_t r1,r2,r3,r4;
uint8_t y1,y2;
uint8_t oldNewToggle = 0;

setup();
loadGFX();
activate2X();


while(exitFlag == 0)
	{
	kernelNextEvent();
	if(kernelEventData.type == kernelEvent(key.PRESSED))
		{
		if(kernelEventData.key.raw == 146) //esc
			{
			return 0;
			}
		if(kernelEventData.key.raw == 0x81) //F1
			{
			oldNewToggle = (oldNewToggle==0?1:0);
			textGotoXY(0,24);
			if(oldNewToggle==1) 
			{
				bitmapSetColor(0);
				textPrint("[F1 toggle] new 2x Lines");
				bitmapSetCLUT(1);
			}
			else 
				{
				bitmapSetColor(0);
				textPrint("[F1 toggle] old Lines    ");
				bitmapSetCLUT(0);
				}
			}
		}

		
	
	//line drawing part
	r1 = randomRead()%320; //for x1
	r2 = randomRead()%320; //for x2
	r3 = randomRead();     //for y1 and y2 split
	r4 = randomRead()%255; //for color
	
	y1 = HIGH_BYTE(r3)%191;
	y2 = LOW_BYTE(r3)%191;

	if(oldNewToggle == 0)
	{
		bitmapSetColor(LOW_BYTE(r4));
		bitmapLine(r1,y1,r2,y2);
	}
	else drawNewLine(r1,r2,y1,y2,LOW_BYTE(r4));
	}

return 0;}

}