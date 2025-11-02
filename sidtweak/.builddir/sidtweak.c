#include "D:\F256\llvm-mos\code\sidtweak\.builddir\trampoline.h"

#define F256LIB_IMPLEMENTATION
#include "f256lib.h"
#include "../src/musid.h" //sid chip
#include "../src/muUtils.h" //utils
#include "../src/mudispatch.h" //dispatch
#include "../src/muMidi.h" //midi
#include "../src/muMIDIin.h" //midi in
#include "../src/textUI.h" //midi in


EMBED(pianopal, "../assets/piano.pal", 0x30000); //1kb
EMBED(keys, "../assets/piano.raw", 0x30400); //



void title(void);
void setup(void);
void resetSID(void);


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
	
	title();
}

void resetSID()
{
	clearSIDRegisters();
	setMonoSID();
	prepSIDinstruments();
}

void title()
{
	textGotoXY(0,0);textSetColor(0x0F,0x03);textPrint("SID Tweak v0.1");
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



int main(int argc, char *argv[]) {
	midiInData gMID;
	
	setup();
	resetSID();
	resetMID(&gMID);
	
	printInstrumentHeaders();
	updateValues();
	
	while(true)
	{
	dealMIDIIn(&gMID);
	}
	
	hitspace();
	return 0;
}
