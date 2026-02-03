#include "D:\F256\llvm-mos\code\float\.builddir\trampoline.h"

#define F256LIB_IMPLEMENTATION
#include "f256lib.h"
#include "..\src\muUtils.h"
#include "..\src\fp_module.h"




#define CENTERX 160
#define CENTERY 150
#define RADIUS 100.0f
#define TOTALINDEX 60


#define PAL_BASE         0x10000
#define SPR_THNG_BASE    0x10800 //sprite 1
#define SPR_SIZE  16
#define SPR_SIZE_SQUARED  0x100

#define VKY_SP0_CTRL     0xD900 //Sprite #0’s control register
#define VKY_SP0_AD_L     0xD901 // Sprite #0’s pixel data address register
#define VKY_SP0_AD_M     0xD902
#define VKY_SP0_AD_H     0xD903
#define VKY_SP0_POS_X_L  0xD904 // Sprite #0’s X position register
#define VKY_SP0_POS_X_H  0xD905
#define VKY_SP0_POS_Y_L  0xD906 // Sprite #0’s Y position register
#define VKY_SP0_POS_Y_H  0xD907


EMBED(palgrudge, "../assets/grudge.pal", 0x10000);//1kb
EMBED(thing, "../assets/thing.bin", 0x10800);//1kb


const float sinTable[60]={
    0.0f, 0.104528f, 0.207912f, 0.309017f, 0.406737f, 0.5f, 0.587785f, 0.669131f,
    0.743145f, 0.809017f, 0.866025f, 0.913545f, 0.951057f, 0.978148f, 0.994522f,
    1.0f, 0.994522f, 0.978148f, 0.951057f, 0.913545f, 0.866025f, 0.809017f,
    0.743145f, 0.669131f, 0.587785f, 0.5f, 0.406737f, 0.309017f, 0.207912f,
    0.104528f, 1.23e-16f, -0.104530f, -0.207910f, -0.309020f, -0.406740f,
    -0.5f, -0.587790f, -0.669130f, -0.743140f, -0.809020f, -0.866030f,
    -0.913550f, -0.951060f, -0.978150f, -0.994520f, -1.0f, -0.994520f,
    -0.978150f, -0.951060f, -0.913550f, -0.866030f, -0.809020f, -0.743140f,
    -0.669130f, -0.587790f, -0.5f, -0.406740f, -0.309020f, -0.207910f,
    -0.104530f
};

int main(int argc, char *argv[]) {
	
//sets the gfx vicky mode
POKE(MMU_IO_CTRL, 0x00);
// XXX GAMMA  SPRITE   TILE  | BITMAP  GRAPH  OVRLY  TEXT
POKE(VKY_MSTR_CTRL_0, 0b00101111); //sprite,graph,overlay,text
// XXX XXX  FON_SET FON_OVLY | MON_SLP DBL_Y  DBL_X  CLK_70
POKE(VKY_MSTR_CTRL_1, 0b00010000); //font overlay, double height text, 320x240 at 60 Hz;


// Set up Color lookup table 0
POKE(MMU_IO_CTRL,1);  //MMU I/O to page 1
for(uint16_t c=0;c<1023;c++)
{
	POKE(VKY_GR_CLUT_0+c, FAR_PEEK(PAL_BASE+c));
}
POKE(MMU_IO_CTRL,0);
	
//Sprite setup
spriteDefine(0, SPR_THNG_BASE, SPR_SIZE, 0, 0); //information for the vicky
spriteSetVisible(0, true);
spriteSetPosition(0,CENTERX,CENTERY);
	

float fx,fy; //float based x and y components to add from a center position (displacement vector), will be computed every loop
uint8_t mode = 0;
//mode 0=uses classic C arithmetics, non-accelerated
//mode 1=uses 2x core float block in fpga,
//mode 2=uses 2x core with asm optimization

//indices to cycle around the sinus look up table. S = sine, C =cosine
uint8_t indexS = 0;
uint8_t indexC = 15;

int16_t x=10,y=0; //offset from center, these will be sent to set the sprite position


textGotoXY(0,0);textPrint("Regular C float products                   ");
POKE(0xD600,0x98);

while(true)
	{
	
	//textGotoXY(x,y);textPrint(" ");
	switch(mode)
		{
		case 0: //mode 0=uses classic C arithmetics, non-accelerated
			fx = sinTable[indexC++]*RADIUS;
			fy = sinTable[indexS++]*RADIUS;
			if(indexC==TOTALINDEX)indexC=0;
			if(indexS==TOTALINDEX)indexS=0;

			x = (uint16_t)((int16_t)CENTERX + (int16_t)fx);
			y = (uint16_t)((int16_t)CENTERY + (int16_t)fy);
			break;
		case 1: //mode 1=uses 2x core float block in fpga,
			fx = mathFloatMul(sinTable[indexC++],RADIUS);
			fy = mathFloatMul(sinTable[indexS++],RADIUS);
			if(indexC==TOTALINDEX)indexC=0;
			if(indexS==TOTALINDEX)indexS=0;

			x = CENTERX + mathFloatToInt16(fx);
			y = CENTERY + mathFloatToInt16(fy);
			break;/*
		case 2: //mode 2=uses 2x core with asm optimization
	
			break;
			*/
		}
	POKE(0xD600,indexS&0x1F);POKE(0xD600,(indexS&0xF0)>>4);
	spriteSetPosition(0,x,y);
	kernelNextEvent();
	if(kernelEventData.type == kernelEvent(key.PRESSED))
		{
			switch(mode)
			{
				case 0: //switch to regular float multiplication
					mode = 1;
					textGotoXY(0,0);textPrint("2x core float block                        ");
					break;
				case 1: //switch to optimizing with asm macros with 2x core float block
					mode =0;
					textGotoXY(0,0);textPrint("Regular C float products                   ");
					break;
					/*
				case 2: //switch to 2x core float block
					mode = 0;
					textGotoXY(0,0);textPrint("Regular C float products                   ");textPrint("2x core float block with asm optimization  ");
					break;
					*/
			}
		}
	}

return 0;
}
