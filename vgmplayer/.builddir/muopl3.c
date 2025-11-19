#include "D:\F256\llvm-mos\code\vgmplayer\.builddir\trampoline.h"

#include "f256lib.h"
#include "../src/muopl3.h"
#include "../src/muUtils.h"
	
opl3I opl3_instrument_defs[18]; //used to store instrument definitions while playback occurs!

//these are used the build the base frequencies for notes of a full octave (12 semi-tones in equal temperament). read the docs because it's not 1:1 to frequency in Hz here.
const uint16_t opl3_fnums[] = {0x205, 0x223, 0x244, 0x267, 0x28B, 0x2B2,
						       0x2DB, 0x306, 0x334, 0x365, 0x399, 0x3CF};
	
uint8_t reverseChan(uint8_t inject)
{
	if(inject>0x0F) return 6 + inject - 0x10;
	else if(inject>0x07) return 3 + inject - 0x08;
	else return inject;
}
void opl3_initialize() {
	uint8_t i;
	
	//chip wide operators
	opl3_write(OPL_EN, 0x20); //chip wide reset
	opl3_write(OPL_T1, 0x00);
	opl3_write(OPL_T2, 0x00);
	opl3_write(OPL_FOE, 0x00); //four operator enable
	opl3_write(OPL_OPL3, 0x01); //enable opl3
	opl3_write(OPL_CSW, 0x00); //composite sine wave mode off
	opl3_write(OPL_PERC, 0x00); //no vibrato depth, trem depth and turn perc mode all off
	
	//channel wide settings
	for(i=0;i<9;i++) {
		opl3_write((uint16_t)OPL_CH_FEED |  (uint16_t)i,   0x30); //channels 0 to 8
	}
	for(i=0;i<9;i++) {
		opl3_write(0x0100 | (uint16_t)OPL_CH_FEED |  (uint16_t)i,   0x30); //channels 0 to 8
	}
	opl3_quietAll();
}
void opl3_quietAll()
{
	uint8_t channel;
	for(channel=0;channel<9;channel++) opl3_write(OPL_CH_KBF_HI | channel, 0x00); //channels 0 to 8
	for(channel=0;channel<9;channel++) opl3_write(0x0100 | (uint16_t)OPL_CH_KBF_HI | (uint16_t)channel, 0x00); //channels 9 to 17
}

void opl3_setInstrument(struct opl3Instrument inst, uint8_t chan)
{
	uint16_t highb=0x0000;
	
	uint8_t offset= 0x00;
	
	offset += chan;
	if(chan>2) offset += 0x05;
	if(chan>5) offset += 0x05;
	if(chan>8)
	{
		offset -= 19;
		highb=0x0100;
	}
	if(chan>11) offset += 0x05;
	if(chan>14) offset += 0x05;
	
	opl3_write(highb | (uint16_t)OPL_OP_TVSKF   | (uint16_t)offset, inst.OP1_TVSKF);
	opl3_write(highb | (uint16_t)OPL_OP_TVSKF   | (uint16_t)(offset+ 0x03), inst.OP2_TVSKF);
	
	opl3_write(highb | (uint16_t)OPL_OP_KSLVOL  |  (uint16_t)offset, inst.OP1_KSLVOL);
	opl3_write(highb | (uint16_t)OPL_OP_KSLVOL  | (uint16_t)(offset+ 0x03), inst.OP2_KSLVOL);
	
	opl3_write(highb | (uint16_t)OPL_OP_AD      |  (uint16_t)offset, inst.OP1_AD);
	opl3_write(highb | (uint16_t)OPL_OP_AD      | (uint16_t)(offset+ 0x03), inst.OP2_AD);
	
	opl3_write(highb | (uint16_t)OPL_OP_SR      |  (uint16_t)offset, inst.OP1_SR);
	opl3_write(highb | (uint16_t)OPL_OP_SR      | (uint16_t)(offset+ 0x03), inst.OP2_SR);
	
	opl3_write(highb | (uint16_t)OPL_OP_WAV     |  (uint16_t)offset, inst.OP1_WAV);
	opl3_write(highb | (uint16_t)OPL_OP_WAV     | (uint16_t)(offset+ 0x03), inst.OP2_WAV);
}

void opl3_setInstrumentAllChannels(uint8_t which)
{
	uint8_t c;
	
	opl3_write(OPL_PERC, opl3_instrument_defs[which].VT_DEPTH); //chip wide trem,vib,perc mode
	
	for(c=0;c<9;c++)
	{
	opl3_setInstrument(opl3_instrument_defs[which],c);
	opl3_write((uint16_t)OPL_CH_FEED | (uint16_t)c,   opl3_instrument_defs[which].CHAN_FEED);
	}
	for(c=0;c<9;c++)
	{
	opl3_setInstrument(opl3_instrument_defs[which],c+9);
	opl3_write(0x0100 | (uint16_t)OPL_CH_FEED | (uint16_t)c,   opl3_instrument_defs[which].CHAN_FEED);
	}
}

void opl3_shadow(uint8_t offset, uint8_t value, uint8_t hinb)
{
uint8_t chan=0;
uint8_t temp =0;

/*
if (hinb == 1) {
	chan = 9; // port 1 shift
	}

switch(offset)
{
	case 0x20 ... 0x22:
	case 0x28 ... 0x2A:
	case 0x30 ... 0x32:
		temp = offset-0x20;
		chan+=reverseChan(temp);
		opl3_instrument_defs[chan].OP2_TVSKF = value;
		textGotoXY(0,chan);printf("%02x",value);
		break;
	case 0x23 ... 0x25:
	case 0x2B ... 0x2D:
	case 0x33 ... 0x35:
		temp = offset-0x23;
		chan+=reverseChan(temp);
		opl3_instrument_defs[chan].OP1_TVSKF = value;
		textGotoXY(2,chan);printf("%02x",value);
		break;
	
	case 0x40 ... 0x42:
	case 0x48 ... 0x4A:
	case 0x50 ... 0x52:
		temp = offset-0x40;
		chan+=reverseChan(temp);
		opl3_instrument_defs[chan].OP2_KSLVOL = value;
		textGotoXY(4,chan);printf("%02x",value);
		break;
	case 0x43 ... 0x45:
	case 0x4B ... 0x4D:
	case 0x53 ... 0x55:
		temp = offset-0x43;
		chan+=reverseChan(temp);
		opl3_instrument_defs[chan].OP1_KSLVOL = value;
		textGotoXY(6,chan);printf("%02x",value);
		break;
	
	case 0x60 ... 0x62:
	case 0x68 ... 0x6A:
	case 0x70 ... 0x72:
		temp = offset-0x60;
		chan+=reverseChan(temp);
		opl3_instrument_defs[chan].OP2_AD = value;
		textGotoXY(8,chan);printf("%02x",value);
		break;
	case 0x63 ... 0x65:
	case 0x6B ... 0x6D:
	case 0x73 ... 0x75:
		temp = offset-0x63;
		chan+=reverseChan(temp);
		opl3_instrument_defs[chan].OP1_AD = value;
		textGotoXY(10,chan);printf("%02x",value);
		break;
	
	case 0x80 ... 0x82:
	case 0x88 ... 0x8A:
	case 0x90 ... 0x92:
		temp = offset-0x80;
		chan+=reverseChan(temp);
		opl3_instrument_defs[chan].OP2_SR = value;
		textGotoXY(12,chan);printf("%02x",value);
		break;
	case 0x83 ... 0x85:
	case 0x8B ... 0x8D:
	case 0x93 ... 0x95:
		temp = offset-0x83;
		chan+=reverseChan(temp);
		opl3_instrument_defs[chan].OP1_SR = value;
		textGotoXY(14,chan);printf("%02x",value);
		break;
	
	case 0xE0 ... 0xE2:
	case 0xE8 ... 0xEA:
	case 0xF0 ... 0xF2:
		temp = offset-0xE0;
		chan+=reverseChan(temp);
		opl3_instrument_defs[chan].OP2_WAV = value;
		textGotoXY(16,chan);printf("%02x",value);
		break;
	case 0xE3 ... 0xE5:
	case 0xEB ... 0xED:
	case 0xF3 ... 0xF5:
		temp = offset-0xE3;
		chan+=reverseChan(temp);
		opl3_instrument_defs[chan].OP1_WAV = value;
		textGotoXY(18,chan);printf("%02x",value);
		break;
	
	case 0xC0 ... 0xC8:
		chan += (offset - 0xC0);
		opl3_instrument_defs[chan].CHAN_FEED = value;
		textGotoXY(20,chan);printf("%02x",value);
		break;
	case 0xBD: //is a global thing, could repeat for all instrument?
		opl3_instrument_defs[chan].VT_DEPTH = value;
		//textGotoXY(22,chan);printf("%02x",chan);
		break;
}
*/
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
	uint16_t hb = 0x0000;
	uint8_t reduce = 0;
	if(channel>8)
	{
	hb = 0x0100;
	reduce = 9;
	}
	// Set frequency (low byte)
    opl3_write(hb | (uint16_t)OPL_CH_F_LO | (uint16_t)(channel-reduce), fnum & 0xFF);
    // Set block/frequency (high byte) and enable sound (Key-On) or off depending on onOrOff value
    opl3_write(hb | (uint16_t)OPL_CH_KBF_HI | (uint16_t)(channel-reduce), ((fnum >> 8) & 0x03) | ((uint16_t)block << 2) | (onOrOff?0x20:0x00));

}

/*
void opl3_StageOne()
{
uint8_t i=0;
//chip wide settings:

	opl3_write(OPL_PERC, opl3_fields[0].value<<4); //no vibrato depth, trem depth and turn perc mode all off
	
//channel wide settings
	for(i=0;i<9;i++) {
		opl3_write((uint16_t)OPL_CH_FEED |  (uint16_t)i,   0x30 | opl3_fields[1].value); //channels 0 to 8
	}
	for(i=0;i<9;i++) {
		opl3_write(0x0100 | (uint16_t)OPL_CH_FEED |  (uint16_t)i,   0x30 | opl3_fields[1].value); //channels 0 to 8
	}
	
	for(i=0;i<18;i++) opl3_StageTwo(i);
}
	
void opl3_StageTwo(uint8_t chan)
{
	uint16_t highb=0x0000;
	
	uint8_t offset= 0x00;
	
	offset += chan;
	if(chan>2) offset += 0x05;
	if(chan>5) offset += 0x05;
	if(chan>8)
	{
		offset -= 19;
		highb=0x0100;
	}
	if(chan>11) offset += 0x05;
	if(chan>14) offset += 0x05;
	
	opl3_write(highb | (uint16_t)OPL_OP_TVSKF   | (uint16_t)offset,         (opl3_fields[18].value<<4) | opl3_fields[20].value); //OP2_TVSKF
	opl3_write(highb | (uint16_t)OPL_OP_TVSKF   | (uint16_t)(offset+ 0x03), (opl3_fields[19].value<<4) | opl3_fields[21].value); //OP1_TVSKF
	
	opl3_write(highb | (uint16_t)OPL_OP_KSLVOL  |  (uint16_t)offset,        (opl3_fields[16].value<<6) | (opl3_fields[12].value << 4) | opl3_fields[13].value); //OP2_KSLVOL
	opl3_write(highb | (uint16_t)OPL_OP_KSLVOL  | (uint16_t)(offset+ 0x03), (opl3_fields[17].value<<6) | (opl3_fields[14].value << 4) | opl3_fields[15].value); //OP1_KSLVOL
	
	opl3_write(highb | (uint16_t)OPL_OP_AD      |  (uint16_t)offset,        (opl3_fields[4].value<<4)  | opl3_fields[6].value); //OP2_AD
	opl3_write(highb | (uint16_t)OPL_OP_AD      | (uint16_t)(offset+ 0x03), (opl3_fields[5].value<<4)  | opl3_fields[7].value); //OP1_AD
	
	opl3_write(highb | (uint16_t)OPL_OP_SR      |  (uint16_t)offset,        (opl3_fields[8].value<<4)  | opl3_fields[10].value); //OP2_SR
	opl3_write(highb | (uint16_t)OPL_OP_SR      | (uint16_t)(offset+ 0x03), (opl3_fields[9].value<<4)  | opl3_fields[11].value); //OP1_SR
	
	opl3_write(highb | (uint16_t)OPL_OP_WAV     |  (uint16_t)offset,        (opl3_fields[2].value<<4)); //OP2_WAV
	opl3_write(highb | (uint16_t)OPL_OP_WAV     | (uint16_t)(offset+ 0x03), (opl3_fields[3].value<<4)); //OP1_WAV
}
*/
