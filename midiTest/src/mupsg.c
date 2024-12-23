#include "f256lib.h"
#include "../src/mupsg.h"

void psgNoteOn(uint8_t loByte, uint8_t hiByte)
{
	POKE(PSG_ADDR,0x94);
	POKE(PSG_ADDR,loByte);
	POKE(PSG_ADDR,hiByte);
}

void psgNoteOff()
{
	POKE(PSG_ADDR,0x9F);
}

	
}