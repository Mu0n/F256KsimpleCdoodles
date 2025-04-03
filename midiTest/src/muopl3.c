#include "f256lib.h"
#include "../src/muopl3.h"


const char *opl3_instrument_names[] = {
	"Twang",
	"Fuzz Guitar",
	"Double Bass",
	"Round Square Lead",
	"Wuzzle",
	"Brassy",
	"Cheap piano",
	"Long sustain bass",
	"Ac. Guitar"
};
	
opl3I opl3_instrument_defs[] = {
	{.OP2_TVSKF=0x21,.OP1_TVSKF=0xC0,.OP2_KSLVOL=0x01,.OP1_KSLVOL=0x15,.OP2_AD=0xF4,.OP1_AD=0xF2,.OP2_SR=0x25,.OP1_SR=0x40,.OP2_WAV=0x04,.OP1_WAV=0x07},
	{.OP2_TVSKF=0x72,.OP1_TVSKF=0x71,.OP2_KSLVOL=0x00,.OP1_KSLVOL=0x48,.OP2_AD=0xF2,.OP1_AD=0xF1,.OP2_SR=0x27,.OP1_SR=0x53,.OP2_WAV=0x02,.OP1_WAV=0x00},
	{.OP2_TVSKF=0x23,.OP1_TVSKF=0x21,.OP2_KSLVOL=0x80,.OP1_KSLVOL=0x4D,.OP2_AD=0x72,.OP1_AD=0x71,.OP2_SR=0x06,.OP1_SR=0x12,.OP2_WAV=0x00,.OP1_WAV=0x01},
	{.OP2_TVSKF=0x21,.OP1_TVSKF=0x22,.OP2_KSLVOL=0x00,.OP1_KSLVOL=0x59,.OP2_AD=0xFF,.OP1_AD=0xFF,.OP2_SR=0x0F,.OP1_SR=0x03,.OP2_WAV=0x00,.OP1_WAV=0x02},
	{.OP2_TVSKF=0x00,.OP1_TVSKF=0x50,.OP2_KSLVOL=0x07,.OP1_KSLVOL=0x03,.OP2_AD=0xFF,.OP1_AD=0xFF,.OP2_SR=0x02,.OP1_SR=0x02,.OP2_WAV=0x00,.OP1_WAV=0x01},
	{.OP2_TVSKF=0x21,.OP1_TVSKF=0x21,.OP2_KSLVOL=0x80,.OP1_KSLVOL=0x8E,.OP2_AD=0x90,.OP1_AD=0xBB,.OP2_SR=0x0A,.OP1_SR=0x29,.OP2_WAV=0x00,.OP1_WAV=0x00},
	{.OP2_TVSKF=0x01,.OP1_TVSKF=0x07,.OP2_KSLVOL=0x48,.OP1_KSLVOL=0x1F,.OP2_AD=0xF5,.OP1_AD=0xF5,.OP2_SR=0xFA,.OP1_SR=0xFA,.OP2_WAV=0x00,.OP1_WAV=0x00},
	{.OP2_TVSKF=0x13,.OP1_TVSKF=0x00,.OP2_KSLVOL=0x00,.OP1_KSLVOL=0x10,.OP2_AD=0xF2,.OP1_AD=0xF2,.OP2_SR=0x72,.OP1_SR=0x72,.OP2_WAV=0x00,.OP1_WAV=0x00},
	{.OP2_TVSKF=0x11,.OP1_TVSKF=0x03,.OP2_KSLVOL=0x00,.OP1_KSLVOL=0x54,.OP2_AD=0xF1,.OP1_AD=0xF3,.OP2_SR=0xE7,.OP1_SR=0x9A,.OP2_WAV=0x00,.OP1_WAV=0x01}
};

const uint8_t opl3_instrumentsSize = 9;

//these are used the build the base frequencies for notes of a full octave (12 semi-tones in equal temperament). read the docs because it's not 1:1 to frequency in Hz here.
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

void opl3_setInstrument(struct opl3Instrument inst, uint8_t chan)
{ 
	uint8_t offset= chan + 0x00;
	if(chan>2) offset += 0x05;
	if(chan>5) offset += 0x05;
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
void opl3_setInstrumentAllChannels(uint8_t which)
{
	uint8_t c;
	
	for(c=0;c<9;c++)
	{
	opl3_setInstrument(opl3_instrument_defs[which],c);
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

}