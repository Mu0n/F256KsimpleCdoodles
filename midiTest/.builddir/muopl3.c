#include "D:\F256\llvm-mos\code\midiTest\.builddir\trampoline.h"

#include "f256lib.h"
#include "../src/muopl3.h"

const uint16_t opl3_fnums[] = {0x205, 0x223, 0x244, 0x267, 0x28B, 0x2B2,
						       0x2DB, 0x306, 0x334, 0x365, 0x399, 0x3CF};
	
void opl3_initialize() {
	uint8_t i;
	
	//chip wide operators
	opl3_write(OPL_EN, 0x20); //chip wide reset
	opl3_write(OPL_T1, 0x00);
	opl3_write(OPL_T2, 0x00);
	opl3_write(OPL_FOE, 0x00); //four operator enable
	opl3_write(OPL_OPL3, 0x01); //enable opl3
	opl3_write(OPL_CSW, 0x00); //composite sine wave mode off
	//channel wide settings
	for(i=0;i<9;i++)
		{
		opl3_write(OPL_CH_FEED + i,   0x30);
		}
	
	opl3_quietAll();

	
}
void opl3_quietAll()
{
	uint8_t channel;
	for(channel=0;channel<9;channel++)
		{
		opl3_write(OPL_CH_KBF_HI | channel, 0x00);
		}
}

void opl3_setInstrument(struct opl3Instrument inst)
{
	uint8_t offset= inst.chan + 0x00;
	if(inst.chan>2) offset += 0x05;
	if(inst.chan>5) offset += 0x05;
	opl3_write(OPL_OP_TVSKF   |  offset, inst.OP1_TVSKF);
	opl3_write(OPL_OP_TVSKF   | (offset+ 0x03), inst.OP2_TVSKF);
	
	opl3_write(OPL_OP_KSLVOL  |  offset, inst.OP1_KSLVOL);
	opl3_write(OPL_OP_KSLVOL  | (offset+ 0x03), inst.OP2_KSLVOL);
	
	opl3_write(OPL_OP_AD      |  offset, inst.OP1_AD);
	opl3_write(OPL_OP_AD      | (offset+ 0x03), inst.OP2_AD);
	
	opl3_write(OPL_OP_SR      |  offset, inst.OP1_SR);
	opl3_write(OPL_OP_SR      | (offset+ 0x03), inst.OP2_SR);
	
	opl3_write(OPL_OP_WAV     |  offset, inst.OP1_WAV);
	opl3_write(OPL_OP_WAV     | (offset+ 0x03), inst.OP2_WAV);
}

void opl3_setDefaultInstruments()
{
	uint8_t c;
	opl3I inst1;
	
    inst1.OP1_TVSKF  = 0xC0; inst1.OP2_TVSKF  = 0x21;
	inst1.OP1_KSLVOL = 0x15; inst1.OP2_KSLVOL = 0x01;
	inst1.OP1_AD     = 0xF2; inst1.OP2_AD     = 0xF4;
	inst1.OP1_SR     = 0x40; inst1.OP2_SR     = 0x25;
	inst1.OP1_WAV    = 0x07; inst1.OP2_WAV    = 0x04;
	
	for(c=0;c<9;c++)
	{
	inst1.chan = c;
	opl3_setInstrument(inst1);
	}
}

// Function to write a value to a YMF262 register
void opl3_write(uint16_t address, uint8_t value) {
    if (address < 0x100) {
        // Address in range 0x000 - 0x0FF
		POKE(OPL_ADDR_L, address);
    } else {
        // Address in range 0x100 - 0x1FF
		POKE(OPL_ADDR_H,(address & 0xFF));
    }
    // Write the value to OPL_DATA
	POKE(OPL_DATA, value);
}


// Function to play a note. Pick one of 12 tones from opl3_fnums, pick your 0-7 octave 'block' and duration in frames
void opl3_note(uint8_t channel, uint16_t fnum, uint8_t block, bool onOrOff) {
	// Set frequency (low byte)
    opl3_write(OPL_CH_F_LO | channel, fnum & 0xFF);
    // Set block/frequency (high byte) and enable sound (Key-On) or off depending on onOrOff value
    opl3_write(OPL_CH_KBF_HI | channel, ((fnum >> 8) & 0x03) | ((uint16_t)block << 2) | (onOrOff?0x20:0x00));

}

