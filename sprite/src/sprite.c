
//DEFINES
#define F256LIB_IMPLEMENTATION

#define PAL_BASE    0x10000
#define SPR_F1      0x14000
#define SPR_F2      0x14900
#define SPR_F3      0x15200
#define SPR_F4      0x15B00
#define SPR_F5      0x16400

#define TIMER_FRAMES 0
#define TIMER_SECONDS 1
#define TIMER_MIDI_COOKIE 0


#define VKY_SP0_CTRL  0xD900 //Sprite #0’s control register
#define VKY_SP0_AD_L  0xD901 // Sprite #0’s pixel data address register
#define VKY_SP0_AD_M     0xD902
#define VKY_SP0_AD_H     0xD903
#define VKY_SP0_POS_X_L  0xD904 // Sprite #0’s X position register
#define VKY_SP0_POS_X_H  0xD905
#define VKY_SP0_POS_Y_L  0xD906 // Sprite #0’s Y position register
#define VKY_SP0_POS_Y_H  0xD907
#define SPR_CLUT_COLORS       32

//INCLUDES
#include "f256lib.h"
#include <stdlib.h>
#include "../src/muUtils.h" //contains helper functions I often use

EMBED(palhuman2, "../assets/backg.pal", 0x10000);
EMBED(peon1, "../assets/peon1.data", 0x14000);
EMBED(peon2, "../assets/peon2.data", 0x14900);
EMBED(peon3, "../assets/peon3.data", 0x15200);
EMBED(peon4, "../assets/peon4.data", 0x15B00);
EMBED(peon5, "../assets/peon5.data", 0x16400);
EMBED(backg, "../assets/backg.raw", 0x30000);

//GLOBALS

//FUNCTION PROTOTYPES
void setup(void);


void hitspace()
{
	bool exitFlag = false;
	
	while(exitFlag == false)
	{
			kernelNextEvent();
			if(kernelEventData.type == kernelEvent(key.PRESSED))
			{
				switch(kernelEventData.key.raw)
				{
					case 148: //enter
					case 32: //space
						exitFlag = true;
						break;
				}
			}
	}
}


void setup()
{
	uint32_t c;
	
	POKE(MMU_IO_CTRL, 0x00);
	// XXX GAMMA  SPRITE   TILE  | BITMAP  GRAPH  OVRLY  TEXT
	POKE(VKY_MSTR_CTRL_0, 0b00101100); //sprite,graph,overlay,text
	// XXX XXX  FON_SET FON_OVLY | MON_SLP DBL_Y  DBL_X  CLK_70
	POKE(VKY_MSTR_CTRL_1, 0b00000100); //font overlay, double height text, 320x240 at 60 Hz;
	POKE(VKY_LAYER_CTRL_0, 0b00000001); //bitmap 1 in layer 0, bitmap 0 in layer 1
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

	
	POKE(MMU_IO_CTRL,0);
	bitmapSetActive(0);
	bitmapSetAddress(0,0x30000);
	bitmapSetCLUT(0);
	
	bitmapSetVisible(0,true);
	bitmapSetVisible(1,false);
	bitmapSetVisible(2,false);
	
	/*
	POKE(0xD900, 0b00000001); //size 1: 24x24, layer 0, lut 0, enabled
	POKEA(0xD901, SPR_F1);
	POKEW(0xD904, 45);
	POKEW(0xD906, 69);
*/	

	spriteDefine(0,SPR_F1,24,0,0);
	spriteSetPosition(0,200,116);
	spriteSetVisible(0,true);


	/*
#define VKY_SP0_CTRL  	 0xD900 //Sprite #0’s control register
#define VKY_SP0_AD_L 	 0xD901 // Sprite #0’s pixel data address register
#define VKY_SP0_AD_M     0xD902
#define VKY_SP0_AD_H     0xD903
#define VKY_SP0_POS_X_L  0xD904 // Sprite #0’s X position register
#define VKY_SP0_POS_X_H  0xD905
#define VKY_SP0_POS_Y_L  0xD906 // Sprite #0’s Y position register
#define VKY_SP0_POS_Y_H  0xD907
*/

	POKE(MMU_IO_CTRL, 0x00);
	
}
int main(int argc, char *argv[]) {
	bool exitFlag=false;
	uint32_t frame[5] = {SPR_F1, SPR_F2, SPR_F3, SPR_F4, SPR_F5};
	uint8_t index=0;
	
	
	setup();
	
	while(!exitFlag)
		{
			hitspace();		
			index++;
			if(index>4) index=0;
			POKEA(VKY_SP0_AD_L, frame[index]);
			
			
		}
	return 0;}
