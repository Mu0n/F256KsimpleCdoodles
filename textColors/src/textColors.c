#define F256LIB_IMPLEMENTATION
#include "f256lib.h"

#include "../src/muUtils.h"


void oldText()
{
	uint8_t x=0, y=0;

POKE(VKY_MSTR_CTRL_0, 0b00100111);
POKE(VKY_MSTR_CTRL_1, 0b00010000); //font overlay, double height text, 320x240 at 60 Hz;

textSetColor(0xF,0);	
textClear();
textPrint("Let's see those text colors ya? [F1] Toggle old/new [F3] Double Height toggle");

	
	for(x=0; x<16; x++)
	{
		textSetColor(x,0);textGotoXY(0+x*5,1);printf(" %02d  ",x);
	}
	
	
	for(x=0; x<16; x++)
	{
		for(y=0; y<16; y++)
		{
			textSetColor(x,y);
			textGotoXY(0+x*5,2+y);textPrint("@#3E ");
		}
	}
}

void newText()
{	
uint8_t buf=0;
textSetColor(3,4);	
textClear();

POKE(MMU_IO_CTRL, 0b01000000);

POKE(VKY_MSTR_CTRL_0, 0b00000111);
POKE(VKY_MSTR_CTRL_1, 0b11010000);

POKE(0xD300, 0x01);//mem text enable

POKEA(0xD304, 0x10000);//start text mem
for(uint16_t x = 0; x<0x2580; x+=2)
{
	uint16_t val = (x/2)%256;
	uint32_t addr = 0x00010000+(uint32_t)x;
    FAR_POKE(addr   ,(uint8_t)val);
    FAR_POKE(addr+(uint32_t)1,0);
}
POKEA(0xD308, 0x20000);//start textcol mem
for(uint16_t x = 0; x<0x2580; x+=2)
{
	uint16_t val = (x/2)%256;
	uint32_t addr = 0x00020000+(uint32_t)x;
    FAR_POKE(addr            ,(uint8_t)val); //bg
    FAR_POKE(addr+(uint32_t)1,(uint8_t)val); //fg 
}

POKE(MMU_IO_CTRL, 0b00001000); //io page 100 = 4 set the LUT0

for(uint16_t x = 0; x<0x400; x+=4)
{
	uint16_t val = (x)%256;
	uint16_t lav = 256-val;
    //FG0
    POKE(0xC000+(uint16_t)(x),  (uint8_t)lav);
    POKE(0xC000+(uint16_t)(x+1), 0);
    POKE(0xC000+(uint16_t)(x+2), 0);
    POKE(0xC000+(uint16_t)(x+3), 0);
	    //FG1
    POKE(0xC400+(uint16_t)(x),  0);
    POKE(0xC400+(uint16_t)(x+1), 0);
    POKE(0xC400+(uint16_t)(x+2), (uint8_t)lav);
    POKE(0xC400+(uint16_t)(x+3), 0);
	//BG0
	POKE(0xC800+(uint16_t)(x),   (uint8_t)val);
    POKE(0xC800+(uint16_t)(x+1), (uint8_t)lav);
    POKE(0xC800+(uint16_t)(x+2), (uint8_t)lav);
    POKE(0xC800+(uint16_t)(x+3), 0);
	//BG1
	POKE(0xCC00+(uint16_t)(x),   0);
    POKE(0xCC00+(uint16_t)(x+1), (uint8_t)lav);
    POKE(0xCC00+(uint16_t)(x+2), 0);
    POKE(0xCC00+(uint16_t)(x+3), 0);
}

/*
POKE(MMU_IO_CTRL, 0b00001001); //io page 100 = 5 set the char def

for(uint16_t x = 0; x<256; x+=8)
{
    
    POKE(0xC000+(uint16_t)(x),   0x00);
    POKE(0xC000+(uint16_t)(x+1), 0x1F);
    POKE(0xC000+(uint16_t)(x+2), 0x30);
    POKE(0xC000+(uint16_t)(x+3), 0x3C);
    POKE(0xC000+(uint16_t)(x+4), 0x7C);
    POKE(0xC000+(uint16_t)(x+5), 0x60);
    POKE(0xC000+(uint16_t)(x+6), 0xC0);
    POKE(0xC000+(uint16_t)(x+7), 0xC0);
}
*/


POKE(MMU_IO_CTRL, 0);
}

int main(int argc, char *argv[]) {	
bool oldOrNew = true, regOrDouble = true;

// XXX GAMMA  SPRITE   TILE  | BITMAP  GRAPH  OVRLY  TEXT
POKE(VKY_MSTR_CTRL_0, 0b00100111); //sprite,graph,overlay,text
// XXX XXX  FON_SET FON_OVLY | MON_SLP DBL_Y  DBL_X  CLK_70
POKE(VKY_MSTR_CTRL_1, 0b00010000); //font overlay, double height text, 320x240 at 60 Hz;

	POKE(0xD00D,0x00); //force black graphics background
	POKE(0xD00E,0x00);
	POKE(0xD00F,0x00);
	
POKE(VKY_MSTR_CTRL_1, (PEEK(VKY_MSTR_CTRL_1) & 0xEF) | 0x10);


oldText();
while(true)
	{
	kernelNextEvent();
	if(kernelEventData.type == kernelEvent(key.PRESSED))
		{
		switch(kernelEventData.key.raw)
			{
			case 0x92: //F1
				return 0;
			case 0x81: //F1
				if(oldOrNew) 
					{
					newText();
					oldOrNew = false;
					}
				else 
					{
					oldText();
					oldOrNew = true;
					}
				break;
			case 0x83: //F3
				if(regOrDouble) 
					{
					POKE(MMU_IO_CTRL, 0x00);
					POKE(VKY_MSTR_CTRL_1, 0b00010100); //font overlay, double height text, 320x240 at 60 Hz;
					regOrDouble = false;
					}
				else 
					{
					POKE(MMU_IO_CTRL, 0x00);
					POKE(VKY_MSTR_CTRL_1, 0b00010000); //font overlay, double height text, 320x240 at 60 Hz;
					regOrDouble = true;
					}
				break;
			}
		}
	}

hitspace();

return 0;

}
}