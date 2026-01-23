#include "D:\F256\llvm-mos\code\mustart\.builddir\trampoline.h"

#include "f256lib.h"
#include "../src/muopl3.h"
#include "../src/muUtils.h"
#include "../src/mudispatch.h"
	
opl3I opl3_instrument_defs[18]; //used to store instrument definitions while playback occurs!
uint8_t chip_VT_PERC; //chip wide vibrato tremolo depth, highest 2 MSb
uint8_t chip_OPL3_PAIRS; //checks out which channels are together if detected, set the byte to 0x3F when spreading that instrument throughout.
uint8_t chip_enable; //enable waveforms other than sine bits
uint8_t chip_NOTESEL; //note select and CSW

//these are used the build the base frequencies for notes of a full octave (12 semi-tones in equal temperament). read the docs because it's not 1:1 to frequency in Hz here.
const uint16_t opl3_fnums[] = {0x205, 0x223, 0x244, 0x267, 0x28B, 0x2B2,
						       0x2DB, 0x306, 0x334, 0x365, 0x399, 0x3CF};
	
//sample instruments
opl3I opl3_instrument_defs[] = {
	{.OP2_TVSKF=0x11,.OP1_TVSKF=0x01,.OP2_KSLVOL=0x00,.OP1_KSLVOL=0x1F,.OP2_AD=0xF2,.OP1_AD=0xF1,.OP2_SR=0x74,.OP1_SR=0x53,.OP2_WAV=0x00,.OP1_WAV=0x00,.CHAN_FEED=0x00,.CHAN_FRLO=0x15,.CHAN_FNUM=0x00},
	{.OP2_TVSKF=0x00,.OP1_TVSKF=0x02,.OP2_KSLVOL=0x0F,.OP1_KSLVOL=0x46,.OP2_AD=0xF2,.OP1_AD=0xF4,.OP2_SR=0xF8,.OP1_SR=0xF4,.OP2_WAV=0x01,.OP1_WAV=0x00,.CHAN_FEED=0x02,.CHAN_FRLO=0x29,.CHAN_FNUM=0x00},
	{.OP2_TVSKF=0xb4,.OP1_TVSKF=0x22,.OP2_KSLVOL=0x00,.OP1_KSLVOL=0x44,.OP2_AD=0x52,.OP1_AD=0x55,.OP2_SR=0x02,.OP1_SR=0x02,.OP2_WAV=0x00,.OP1_WAV=0x00,.CHAN_FEED=0x00,.CHAN_FRLO=0x00,.CHAN_FNUM=0x00},
	{.OP2_TVSKF=0x08,.OP1_TVSKF=0x01,.OP2_KSLVOL=0x42,.OP1_KSLVOL=0x10,.OP2_AD=0xF1,.OP1_AD=0xF1,.OP2_SR=0x53,.OP1_SR=0x53,.OP2_WAV=0x00,.OP1_WAV=0x00,.CHAN_FEED=0x08,.CHAN_FRLO=0x31,.CHAN_FNUM=0x00}
};

	
uint8_t reverseChan(uint8_t inject) // returns the channel number from having an gap between current register and base register
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
	for(channel=0;channel<9;channel++)
	{
		//opl3_write(OPL_CH_F_LO   | channel, opl3_instrument_defs[channel].CHAN_FRLO); //channels 0 to 8
		opl3_write(OPL_CH_KBF_HI | channel, 0x00); //channels 0 to 8 remove key on
	
		//textGotoXY(START_X_VIS+60   ,channel+START_HEIGHT_VIS_ALT);printf(" %02x ",opl3_instrument_defs[channel].CHAN_FNUM & 0xDF);
		//opl3_write(OPL_CH_F_LO   | channel, 0x00); //channels 0 to 8
	}
	for(channel=0;channel<9;channel++)
	{
		//opl3_write(0x0100 | (uint16_t)OPL_CH_F_LO   | (uint16_t)channel, opl3_instrument_defs[channel+9].CHAN_FRLO); //channels 9 to 17
		opl3_write(0x0100 | (uint16_t)OPL_CH_KBF_HI | (uint16_t)channel, 0x00); //channels 9 to 17 remove key on
		//textGotoXY(START_X_VIS+60   ,channel+9+START_HEIGHT_VIS_ALT);printf(" %02x ",opl3_instrument_defs[channel+9].CHAN_FNUM & 0xDF);
	
		//opl3_write(0x0100 | (uint16_t)OPL_CH_F_LO   | (uint16_t)channel, 0x00); //channels 9 to 17
	}
	
}

void opl3_setInstrumentAllChannels(uint8_t which, bool steamRoll)
{
	
textSetColor(14,0);
//axes_info(START_HEIGHT_VIS_ALT);
//bitmapSetVisible(0,false);

	opl3_write(OPL_EN, chip_enable); //will decide on the waveform select or sine by default
	//textGotoXY(START_X_VIS+52,START_HEIGHT_VIS_ALT);printf(" %02x ",chip_enable);
	opl3_write(OPL_PERC, chip_VT_PERC & 0xE0); //chip wide trem,vib,perc mode
	//textGotoXY(START_X_VIS+48,START_HEIGHT_VIS_ALT);printf(" %02x ",chip_VT_PERC);
	
	opl3_write(OPL_CSW, chip_NOTESEL);
	//textGotoXY(START_X_VIS+56,START_HEIGHT_VIS_ALT);printf(" %02x ",chip_NOTESEL);
	
	
	for(uint8_t indx=0;indx<18;indx++)
	{
	if(steamRoll)
		{
		opl3_setFeed(opl3_instrument_defs[which].CHAN_FEED, indx);
		opl3_setFrLo(opl3_instrument_defs[which].CHAN_FRLO, indx);
		opl3_setFnum(opl3_instrument_defs[which].CHAN_FNUM & 0xDF, indx);
		opl3_setInstrument(opl3_instrument_defs[which],indx);
		}
	if(steamRoll == false)
		{
		opl3_setFeed(opl3_instrument_defs[indx].CHAN_FEED, indx);
		//opl3_setFrLo(opl3_instrument_defs[indx].CHAN_FRLO, indx);
		//opl3_setFnum(opl3_instrument_defs[indx].CHAN_FNUM & 0xDF, indx);
		if(indx == 7 && chip_VT_PERC>0) //snare - carrier only
			{
			opl3_write(OPL_OP_WAV     |  (uint16_t)0x14, opl3_instrument_defs[indx].OP2_WAV);
			opl3_write(OPL_OP_TVSKF   |  (uint16_t)0x14, opl3_instrument_defs[indx].OP2_TVSKF);
			opl3_write(OPL_OP_KSLVOL  |  (uint16_t)0x14, opl3_instrument_defs[indx].OP2_KSLVOL);
			opl3_write(OPL_OP_AD      |  (uint16_t)0x14, opl3_instrument_defs[indx].OP2_AD);
			opl3_write(OPL_OP_SR      |  (uint16_t)0x14, opl3_instrument_defs[indx].OP2_SR);

			opl3_write(OPL_OP_WAV     |  (uint16_t)0x11, opl3_instrument_defs[indx].OP1_WAV);
			opl3_write(OPL_OP_TVSKF   |  (uint16_t)0x11, opl3_instrument_defs[indx].OP1_TVSKF);
			opl3_write(OPL_OP_KSLVOL  |  (uint16_t)0x11, opl3_instrument_defs[indx].OP1_KSLVOL);
			opl3_write(OPL_OP_AD      |  (uint16_t)0x11, opl3_instrument_defs[indx].OP1_AD);
			opl3_write(OPL_OP_SR      |  (uint16_t)0x11, opl3_instrument_defs[indx].OP1_SR);
			}
		if(indx == 8 && chip_VT_PERC>0 ) // Tom-Tom - carrier only
			{
			opl3_write(OPL_OP_WAV     +  (uint16_t)0x15, opl3_instrument_defs[indx].OP2_WAV);
			opl3_write(OPL_OP_TVSKF   +  (uint16_t)0x15, opl3_instrument_defs[indx].OP2_TVSKF);
			opl3_write(OPL_OP_KSLVOL  +  (uint16_t)0x15, opl3_instrument_defs[indx].OP2_KSLVOL);
			opl3_write(OPL_OP_AD      +  (uint16_t)0x15, opl3_instrument_defs[indx].OP2_AD);
			opl3_write(OPL_OP_SR      +  (uint16_t)0x15, opl3_instrument_defs[indx].OP2_SR);

			opl3_write(OPL_OP_WAV     +  (uint16_t)0x12, opl3_instrument_defs[indx].OP1_WAV);
			opl3_write(OPL_OP_TVSKF   +  (uint16_t)0x12, opl3_instrument_defs[indx].OP1_TVSKF);
			opl3_write(OPL_OP_KSLVOL  +  (uint16_t)0x12, opl3_instrument_defs[indx].OP1_KSLVOL);
			opl3_write(OPL_OP_AD      +  (uint16_t)0x12, opl3_instrument_defs[indx].OP1_AD);
			opl3_write(OPL_OP_SR      +  (uint16_t)0x12, opl3_instrument_defs[indx].OP1_SR);
			}

		else opl3_setInstrument(opl3_instrument_defs[indx],indx);
		}

	
	}
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
	
	
	opl3_write(highb | (uint16_t)OPL_OP_WAV     | (uint16_t)offset,                 inst.OP1_WAV);
	opl3_write(highb | (uint16_t)OPL_OP_WAV     | (uint16_t)offset+ (uint16_t)0x03, inst.OP2_WAV);
	
	opl3_write(highb | (uint16_t)OPL_OP_TVSKF   | (uint16_t)offset,                 inst.OP1_TVSKF);
	opl3_write(highb | (uint16_t)OPL_OP_TVSKF   | (uint16_t)offset+ (uint16_t)0x03, inst.OP2_TVSKF);
	
	opl3_write(highb | (uint16_t)OPL_OP_KSLVOL  | (uint16_t)offset,                 inst.OP1_KSLVOL);
	opl3_write(highb | (uint16_t)OPL_OP_KSLVOL  | (uint16_t)offset+ (uint16_t)0x03, inst.OP2_KSLVOL);
	
	opl3_write(highb | (uint16_t)OPL_OP_AD      | (uint16_t)offset,                 inst.OP1_AD);
	opl3_write(highb | (uint16_t)OPL_OP_AD      | (uint16_t)offset+ (uint16_t)0x03, inst.OP2_AD);
	
	opl3_write(highb | (uint16_t)OPL_OP_SR      | (uint16_t)offset,                 inst.OP1_SR);
	opl3_write(highb | (uint16_t)OPL_OP_SR      | (uint16_t)offset+ (uint16_t)0x03, inst.OP2_SR);

}


void opl3_setFeed(uint8_t val, uint8_t which)
{
	if(which>8)
	{
		opl3_write(0x100 | (uint16_t)OPL_CH_FEED | (uint16_t)which,   val);
		//textGotoXY(START_X_VIS+40,which+START_HEIGHT_VIS_ALT);printf(" %02x ",val);
	}
	else
	{
		opl3_write((uint16_t)OPL_CH_FEED | (uint16_t)which,   val);
		//textGotoXY(START_X_VIS+40,which+START_HEIGHT_VIS_ALT);printf(" %02x ",val);
	}
}
void opl3_setFrLo(uint8_t val, uint8_t which)
{
	if(which>8)
	{
		opl3_write(0x100 | (uint16_t)OPL_CH_F_LO | (uint16_t)which, val);
	}
	else
	{
		opl3_write((uint16_t)OPL_CH_F_LO | (uint16_t)which, val);
	}
}
void opl3_setFnum(uint8_t val, uint8_t which)
{
	if(which>8)
	{
		opl3_write(0x100 | (uint16_t)OPL_CH_KBF_HI | (uint16_t)which, val);
		//textGotoXY(START_X_VIS+44,which+START_HEIGHT_VIS_ALT);printf(" %02x ",val);
	}
	else
	{
		opl3_write((uint16_t)OPL_CH_KBF_HI | (uint16_t)which, val);
		//textGotoXY(START_X_VIS+44,which+START_HEIGHT_VIS_ALT);printf(" %02x ",val);
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

