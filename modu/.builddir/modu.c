#include "D:\F256\llvm-mos\code\modu\.builddir\trampoline.h"

#define F256LIB_IMPLEMENTATION

#define PS2_M_MODE_EN 0xD6E0
#define PS2_M_X_LO    0xD6E2
#define PS2_M_X_HI    0xD6E3
#define PS2_M_Y_LO    0xD6E4
#define PS2_M_Y_HI    0xD6E5

#define PS2_CTRL 0xD640
#define PS2_M_IN 0xD643
#define PS2_STAT 0xD644


#include "f256lib.h"
#include "../src/mumouse.h"
#include "../src/musid.h"
#include "../src/muMidi.h"
#include "../src/mudispatch.h"
#include "../src/moduUI.h"


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

void setup()
{
backgroundSetup();
clearSIDRegisters();
gPtr = malloc(sizeof(globalThings));
resetGlobals(gPtr);
sid_setInstrumentAllChannels(2);
setMonoSID();
prepMouse();
}

int main(int argc, char *argv[]) {
int16_t newX, newY, oldX=0, oldY=0;
bool mPressed = false;
uint8_t lastNote=0;
bool nActive = false;

setup();

while(true)
{
	
	kernelNextEvent();
	if(kernelEventData.type == kernelEvent(mouse.CLICKS))
	{
		//dispatchNote(bool isOn, uint8_t channel, uint8_t note, uint8_t speed, bool wantAlt, uint8_t whichChip, bool isBeat, uint8_t beatChan)
	}
	else if(kernelEventData.type == kernelEvent(mouse.DELTA))
	{
		newX = PEEKW(PS2_M_X_LO)+(int8_t)kernelEventData.mouse.delta.x;
		newY = PEEKW(PS2_M_Y_LO)+(int8_t)kernelEventData.mouse.delta.y;
	
		if(newX<0) newX=0; if(newX>640-16) newX=640-16;
		if(newY<0) newY=0; if(newY>480-16) newY=480-16;
		POKEW(PS2_M_X_LO,newX);
        POKEW(PS2_M_Y_LO,newY);
	
		if(newX == oldX && newY == oldY)
			{
			if((kernelEventData.mouse.delta.buttons&0x01)==0x01 && nActive ==false)
				{
					mPressed=true;
					nActive=true;
					lastNote = newY>>2;
					dispatchNote(true, 0, lastNote, 0x7F, false, 1, false, 0);
				}
			if(mPressed==true && (kernelEventData.mouse.delta.buttons&0x01)==0x00 && nActive==true)
				{
					mPressed=false;
					nActive=false;
					dispatchNote(false, 0, lastNote, 0x7F, false, 1, false, 0);
				}
			}
		else
		{
			oldX = newX;
			oldY = newY;
		}

	}
}


return 0;}

