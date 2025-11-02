#define F256LIB_IMPLEMENTATION
#include "f256lib.h"
#include "../src/musid.h" //sid chip
#include "../src/muUtils.h" //utils
#include "../src/mudispatch.h" //utils


void setup(void);
void resetSID(void);


void setup()
{
	openAllCODEC(); //if the VS1053b is used, this might be necessary for some board revisions	
	//initVS1053MIDI();  //if the VS1053b is used
	
	//wipeBitmapBackground(0x2F,0x2F,0x2F);
	POKE(MMU_IO_CTRL, 0x00);
	// XXX GAMMA  SPRITE   TILE  | BITMAP  GRAPH  OVRLY  TEXT
	POKE(VKY_MSTR_CTRL_0, 0b00000001); //sprite,graph,overlay,text
	// XXX XXX  FON_SET FON_OVLY | MON_SLP DBL_Y  DBL_X  CLK_70
	POKE(VKY_MSTR_CTRL_1, 0b00000100); //font overlay, double height text, 320x240 at 60 Hz;
}

void resetSID()
{
	clearSIDRegisters();
	prepSIDinstruments();
	setMonoSID();
}

int main(int argc, char *argv[]) {
	uint16_t i;
	
	setup();
	resetSID();
	
	printf("testing SID");


	POKE(0xD400,0xF9);POKE(0xD401,0x10);
	POKE(0xD405,0x44);POKE(0xD406,0x44);
	POKE(0xD402,0x44);POKE(0xD403,0x00);
	POKE(0xD418,0x0F);
	POKE(0xD404,0x21);
	
	
	hitspace();

	POKE(0xD404,0x20);
	hitspace();
	return 0;
}
	