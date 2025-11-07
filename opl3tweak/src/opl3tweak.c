#define F256LIB_IMPLEMENTATION
#include "f256lib.h"
#include "../src/muopl3.h"
#include "../src/muUtils.h" //utils
#include "../src/mudispatch.h" //dispatch
#include "../src/muMidi.h" //midi
#include "../src/muMIDIin.h" //midi in
#include "../src/textUI.h" //midi in


EMBED(pianopal, "../assets/piano.pal", 0x30000); //1kb
EMBED(keys, "../assets/piano.raw", 0x30400); //

int8_t exitCode=1; //when = to 0, exit program
bool shiftTog = false;

void setup(void);
void resetOPL(void);
void dealKeyPressed(uint8_t);
void dealKeyb(void);
void goBack(uint8_t);
void goForth(uint8_t);

void setup()
{
	//openAllCODEC(); //if the VS1053b is used, this might be necessary for some board revisions	
	
	//wipeBitmapBackground(0x2F,0x2F,0x2F);
	POKE(MMU_IO_CTRL, 0x00);
	// XXX GAMMA  SPRITE   TILE  | BITMAP  GRAPH  OVRLY  TEXT
	POKE(VKY_MSTR_CTRL_0, 0b00111111); //sprite,graph,overlay,text
	// XXX XXX  FON_SET FON_OVLY | MON_SLP DBL_Y  DBL_X  CLK_70
	POKE(VKY_MSTR_CTRL_1, 0b00000100); //font overlay, double height text, 320x240 at 60 Hz;


	POKE(0xD00D,0x00); //force black graphics background
	POKE(0xD00E,0x00);
	POKE(0xD00F,0x00);
//palette
	POKE(MMU_IO_CTRL,1);  //MMU I/O to page 1
	//prep to copy over the palette to the CLUT

	for(uint16_t c=0;c<1023;c++) 
	{
		POKE(VKY_GR_CLUT_0+c, FAR_PEEK(0x30000+c)); //palette for piano
	}
	POKE(MMU_IO_CTRL,0); //MMU I/O to page 0
	
	bitmapReset();
	bitmapSetActive(0);
	bitmapSetAddress(0,0x30400);
	bitmapSetVisible(0,true);
	resetGlobals(gPtr);
	
	textEnableBackgroundColors(true);
}

void resetOPL()
{
	opl3_initialize();
	opl3_setInstrumentAllChannels(11);
}


void resetMID(midiInData *themidi)
{
themidi->recByte = 0x00; 
themidi->nextIsNote = 0x00;
themidi->nextIsSpeed = 0x00; 
themidi->isHit = 0x00; 
themidi->lastCmd = 0x90; 
themidi->lastNote = 0x00; 
themidi->storedNote = 0x00;	
}
void goForth(uint8_t isVert)
	{
	if(indexUI == MAXUIINDEX) 
		{
		updateHighlight(MAXUIINDEX, 0);
		return;
		}
	else if(isVert) 
		{
		uint8_t newIndex = indexUI+(navWSJumps[indexUI]&0x0F);
		if(newIndex > MAXUIINDEX) newIndex =0;
		updateHighlight(indexUI, newIndex);
		}
	else updateHighlight(indexUI, indexUI+1);
	}

void goBack(uint8_t isVert)
	{
	if(indexUI == 0 && isVert == false) 
		{
		updateHighlight(0, MAXUIINDEX);
		return;
		}
	else if(isVert) 
		{
		uint8_t newIndex = indexUI-((navWSJumps[indexUI]&0xF0)>>4);
		if(newIndex > MAXUIINDEX) newIndex = MAXUIINDEX- (0xFF - newIndex);
		updateHighlight(indexUI, newIndex);
		}
	else updateHighlight(indexUI, indexUI-1);
	}

void updateValPunch(uint8_t valPunch)
{
opl3_fields[indexUI].value = valPunch;
if(indexUI == 0 && valPunch > 7) opl3_fields[indexUI].value = 7;
opl3_fields[indexUI].is_dirty = true;
updateValues();
}

void dealKeyPressed(uint8_t keyRaw)
{
	
	switch(keyRaw)
	{
		case 0x87: //F7
			randomInst();
			break;
		case 146: // top left backspace, meant as reset
			exitCode = 0;
			break;
		case 0x00: // left shift
		case 0x01: // right shift
			shiftTog = true;
			break;
		case 0x93: //tab
			if(shiftTog) goBack(0);
			else goForth(0); 
			break;
		case 0xB6: //up
			goBack(1);
			break;
		case 0xB7: //down
			goForth(1);
			break;
		case 0xB8: //left
			goBack(0);
			break;
		case 0xB9: //right
			goForth(0);
			break;
		case 0x3D: //=+
			opl3_fields[indexUI].value ++;
			if(indexUI == 0 && opl3_fields[indexUI].value>0x07) opl3_fields[indexUI].value = 0x00;
			if(opl3_fields[indexUI].value>0x0F) opl3_fields[indexUI].value = 0x00;
			opl3_fields[indexUI].is_dirty = true;
			updateValues();
			break;
		case 0x2D: //-_
			if(indexUI == 0 && opl3_fields[indexUI].value==0x00) opl3_fields[indexUI].value = 0x07;
			else if(opl3_fields[indexUI].value==0x00) opl3_fields[indexUI].value = 0x0F;
			else opl3_fields[indexUI].value--;
			opl3_fields[indexUI].is_dirty = true;
			updateValues();
			break;
		case 0x30: //0
			updateValPunch(0);
			break;
		case 0x31: //1
			updateValPunch(1);
			break;
		case 0x32: //2
			updateValPunch(2);
			break;
		case 0x33: //3
			updateValPunch(3);
			break;
		case 0x34: //4
			updateValPunch(4);
			break;
		case 0x35: //5
			updateValPunch(5);
			break;
		case 0x36: //6
			updateValPunch(6);
			break;
		case 0x37: //7
			updateValPunch(7);
			break;
		case 0x38: //8
			updateValPunch(8);
			break;
		case 0x39: //9
			updateValPunch(9);
			break;
		case 0x61: //A
			updateValPunch(0xA);
			break;
		case 0x62: //B
			updateValPunch(0xB);
			break;
		case 0x81: //C
			updateValPunch(0xC);
			break;
		case 0x64: //D
			updateValPunch(0xD);
			break;
		case 0x65: //E
			updateValPunch(0xE);
			break;
		case 0x66: //F
			updateValPunch(0xF);
			break;

	}
	//
}

void dealKeyb()
{
	kernelNextEvent();
	if(kernelEventData.type == kernelEvent(key.PRESSED))
			{
			dealKeyPressed(kernelEventData.key.raw);
			}
	else if(kernelEventData.type == kernelEvent(key.RELEASED))
		{
			if(kernelEventData.key.raw == 0x00 || kernelEventData.key.raw == 0x01) shiftTog = false;
		}
}

int main(int argc, char *argv[]) {
	midiInData gMID;
	
	setup();
	resetOPL();
	resetMID(&gMID);
	initSIDFields();
	
	printInstrumentHeaders();
	updateValues();
	
	while(exitCode)
	{		
	dealMIDIIn(&gMID);
	dealKeyb();
	}
	
	resetOPL();
	return 0;
}
	