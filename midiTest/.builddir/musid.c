#include "D:\F256\llvm-mos\code\midiTest\.builddir\trampoline.h"

#include "f256lib.h"
#include "../src/musid.h"


void clearSIDRegisters(void)
{
	uint8_t i;
	for(i=0;i<0x18;i++)
	{
		POKE(SID1+i,0);
		POKE(SID2+i,0);
	}
}

void shutAllSIDVoices(void)
{
	POKE(SID1+SID_VOICE1+SID_CTRL, PEEK(SID1+SID_VOICE1+SID_CTRL) & 0xFE);
	POKE(SID1+SID_VOICE2+SID_CTRL, PEEK(SID1+SID_VOICE2+SID_CTRL) & 0xFE);
	POKE(SID1+SID_VOICE3+SID_CTRL, PEEK(SID1+SID_VOICE3+SID_CTRL) & 0xFE);
	
	POKE(SID2+SID_VOICE1+SID_CTRL, PEEK(SID2+SID_VOICE1+SID_CTRL) & 0xFE);
	POKE(SID2+SID_VOICE2+SID_CTRL, PEEK(SID2+SID_VOICE2+SID_CTRL) & 0xFE);
	POKE(SID2+SID_VOICE3+SID_CTRL, PEEK(SID2+SID_VOICE3+SID_CTRL) & 0xFE);
}
//sidNoteOnOrOff: 1st argument decides the right base address of sid # with its voice #
// 2nd argument contains the control register data (where the instrument wave is defined)
// 3rd argument decides if the gate will be opened up (bool 1) or closed (bool 0)
void sidNoteOnOrOff(uint16_t voice, uint8_t ctrl, bool isOn)
{
	POKE(voice, isOn?(ctrl|0x01):(ctrl&0xFE));
	}
