#include "D:\F256\llvm-mos\code\mousing\.builddir\trampoline.h"

#define F256LIB_IMPLEMENTATION



#include "f256lib.h"
#include "../src/mumouse.h"


void backgroundSetup()
{
	POKE(MMU_IO_CTRL, 0x00);
	// XXX GAMMA  SPRITE   TILE  | BITMAP  GRAPH  OVRLY  TEXT
	POKE(VKY_MSTR_CTRL_0, 0b00101111); //sprite,graph,overlay,text
	// XXX XXX  FON_SET FON_OVLY | MON_SLP DBL_Y  DBL_X  CLK_70
	POKE(VKY_MSTR_CTRL_1, 0b00000100); //font overlay, double height text, 320x240 at 60 Hz;
	POKE(VKY_LAYER_CTRL_0, 0b00000001); //bitmap 1 in layer 0, bitmap 0 in layer 1
	POKE(VKY_LAYER_CTRL_1, 0b00000010); //bitmap 2 in layer 2
	POKE(0xD00D,0x00); //force black graphics background
	POKE(0xD00E,0x00);
	POKE(0xD00F,0x00);

	bitmapSetActive(0);
	bitmapSetCLUT(0);
	
	bitmapSetVisible(0,false);
	bitmapSetVisible(1,false);
	bitmapSetVisible(2,false);
	
}


int main(int argc, char *argv[]) {
int16_t newX, newY;
int8_t boost;
uint32_t clicks=0;


backgroundSetup();
prepMouse();

textGotoXY(10,10);printf("Number of clicks: ");
while(true)
{
	kernelNextEvent();
	if(kernelEventData.type == kernelEvent(mouse.CLICKS))
	{
		textGotoXY(10,11);printf("%ld",++clicks);
	
	}
	else if(kernelEventData.type == kernelEvent(mouse.DELTA))
	{
		if((int8_t)kernelEventData.mouse.delta.x > 4 || (int8_t)kernelEventData.mouse.delta.x < -4) boost = 2;
		else boost = 1;
		newX = PEEKW(PS2_M_X_LO)+boost*(int8_t)kernelEventData.mouse.delta.x;
		if((int8_t)kernelEventData.mouse.delta.y > 4 || (int8_t)kernelEventData.mouse.delta.y < -4) boost = 2;
		else boost = 1;
		newY = PEEKW(PS2_M_Y_LO)+boost*(int8_t)kernelEventData.mouse.delta.y;
	
		if(newX<0) newX=0; if(newX>640-16) newX=640-16;
		if(newY<0) newY=0; if(newY>480-16) newY=480-16;
		POKEW(PS2_M_X_LO,newX);
        POKEW(PS2_M_Y_LO,newY);
	}

}


return 0;}

