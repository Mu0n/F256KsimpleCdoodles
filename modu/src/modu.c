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
#include "../src/muopl3.h"
#include "../src/mupsg.h"


EMBED(gui, "../assets/gui.bin", 0x10000); //4kb
EMBED(pal, "../assets/gui.pal", 0x11000); //1kb

typedef struct buttonUI
{
	int8_t indexUI;
	uint8_t indexSPR;
	int16_t x,y;
	bool isHovered;
	bool isClicked;
	bool isDisabled;
	uint32_t addr; //base sprite gfx address
	uint8_t state; //which sprite gfx to show
	uint8_t size; //8, 16 or 32x32
} buttonUI;

struct buttonUI knob;

void loadSprite(uint8_t s, uint32_t addr, uint8_t frame, uint16_t x, uint16_t y, struct buttonUI *but) 
{
	spriteDefine(s, addr, 8, 0, 0);
	spriteSetPosition(s,x,y);
	spriteSetVisible(s,false);

	but->indexUI = 0;
	but->indexSPR = s;
	but->isHovered = false;
	but->isClicked = false;
	but->isDisabled = false;
	
	but->x = x;
	but->y = y;
	but->addr = addr;
	but->state = frame;
}

void loadGUI()
{
	loadSprite(0, 0x10000 + 56 *8*8, 0, 0x007F, 0x007F, &knob); 
	spriteSetVisible(0,true);
}

void backgroundSetup()
{
	uint16_t c;
	
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

//palette
	POKE(MMU_IO_CTRL,1);  //MMU I/O to page 1
	//prep to copy over the palette to the CLUT
	for(c=0;c<1023;c++) 
	{
		POKE(VKY_GR_CLUT_0+c, FAR_PEEK(0x11000+c)); 
	}
	
	POKE(MMU_IO_CTRL,0); //MMU I/O to page 0

	bitmapSetActive(0);
	bitmapSetCLUT(0);
	
	bitmapClear();
	bitmapSetColor(3);
	bitmapSetVisible(0,true);
	bitmapSetVisible(1,false);
	bitmapSetVisible(2,false);
	
}

void setup()
{
backgroundSetup();
loadGUI();

//set a structure of globals related to sound dispatching
gPtr = malloc(sizeof(globalThings));
resetGlobals(gPtr);	

//SID prep
clearSIDRegisters();
sid_setInstrumentAllChannels(2);
setMonoSID();
	
//Prep PSG stuff
setMonoPSG();

//Prep OPL3 stuff
opl3_initialize();
opl3_setInstrumentAllChannels(0);

prepMouse();
}

void drawRect(int16_t x, int16_t y)
{
		bitmapLine(x,y,x+30,y);
		bitmapLine(x,y+18,x+30,y+18);
		bitmapLine(x,y,x,y+18);
		bitmapLine(x+30,y,x+30,y+18);
}


void checkUIClick(uint16_t newX,uint16_t newY)
{
	if(newX >= knob.x && newX <= (knob.x+8) && newY >= knob.y && newY <= knob.y+8)
	{
		if(knob.isClicked) 
		{
			spriteDefine(knob.indexSPR, 0x10000 + (uint32_t)(56 *8*8), 8, 0, 0);
			knob.isClicked = false;
		}
		else 
		{
			spriteDefine(knob.indexSPR, 0x10000 + (uint32_t)(57 *8*8), 8, 0, 0);
			knob.isClicked = true;
		}
	}
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
			textGotoXY(0,0);printf("%04d %04d",newX,newY);
			textGotoXY(0,1);printf("%04d %04d",knob.x,knob.y);
			if((kernelEventData.mouse.delta.buttons&0x01)==0x01 && nActive ==false)
				{
					mPressed=true;
					checkUIClick(newX>>1,newY>>1);
					/*
					nActive=true;
					lastNote = newY>>2;
					dispatchNote(true, 0, lastNote, 0x7F, false, 3, false, 0);
					*/
				}
			if(mPressed==true && (kernelEventData.mouse.delta.buttons&0x01)==0x00 && nActive==true)
				{
					mPressed=false;
					
					/*
					nActive=false;
					dispatchNote(false, 0, lastNote, 0x7F, false, 3, false, 0);
					*/
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

