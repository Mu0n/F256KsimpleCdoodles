#include "D:\F256\llvm-mos\code\vgmplayer2\.builddir\trampoline.h"

#include "f256lib.h"
#include "../src/muopl3.h"
#include "../src/muUtils.h"
#include "../src/textUI.h"
#include "../src/mudispatch.h"
	
opl3I opl3_instrument_defs[18]; //used to store instrument definitions while playback occurs!
uint8_t chip_VT_PERC; //chip wide vibrato tremolo depth, highest 2 MSb
uint8_t chip_OPL3_PAIRS; //checks out which channels are together if detected, set the byte to 0x3F when spreading that instrument throughout.
uint8_t chip_enable; //enable waveforms other than sine bits
uint8_t chip_NOTESEL; //note select and CSW

//these are used the build the base frequencies for notes of a full octave (12 semi-tones in equal temperament). read the docs because it's not 1:1 to frequency in Hz here.
const uint16_t opl3_fnums[] = {0x205, 0x223, 0x244, 0x267, 0x28B, 0x2B2,
						       0x2DB, 0x306, 0x334, 0x365, 0x399, 0x3CF};
	
uint8_t reverseChan(uint8_t inject) // returns the channel number from having an gap between current register and base register
{
	if(inject>0x0F) return 6 + inject - 0x10;
	else if(inject>0x07) return 3 + inject - 0x08;
	else return inject;
}

void opl3_initialize_defs() {
	chip_VT_PERC = 0;
	chip_OPL3_PAIRS = 0;
	chip_enable = 0;
	for(uint8_t i=0;i<18;i++)
	{
		opl3_instrument_defs[i].OP2_TVSKF=0; //OP2 Carrier, OP1 Modulator
		opl3_instrument_defs[i].OP2_TVSKF=0; //OP2 Carrier, OP1 Modulator
		opl3_instrument_defs[i].OP1_TVSKF=0;
		opl3_instrument_defs[i].OP2_KSLVOL=0;
		opl3_instrument_defs[i].OP1_KSLVOL=0;
		opl3_instrument_defs[i].OP2_AD=0;
		opl3_instrument_defs[i].OP1_AD=0;
		opl3_instrument_defs[i].OP2_SR=0;
		opl3_instrument_defs[i].OP1_SR=0;
		opl3_instrument_defs[i].OP2_WAV=0;
		opl3_instrument_defs[i].OP1_WAV=0;
		opl3_instrument_defs[i].CHAN_FEED=0;
		opl3_instrument_defs[i].CHAN_FRLO=0;
		opl3_instrument_defs[i].CHAN_FNUM=0;
		opl3_instrument_defs[i].KEYHIT = 0x00;
	}
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
		opl3_write(OPL_CH_KBF_HI | channel, opl3_instrument_defs[channel].CHAN_FNUM & 0xDF); //channels 0 to 8 remove key on
	
		//textGotoXY(START_X_VIS+60   ,channel+START_HEIGHT_VIS_ALT);printf(" %02x ",opl3_instrument_defs[channel].CHAN_FNUM & 0xDF);
		//opl3_write(OPL_CH_F_LO   | channel, 0x00); //channels 0 to 8
	}
	for(channel=0;channel<9;channel++)
	{
		//opl3_write(0x0100 | (uint16_t)OPL_CH_F_LO   | (uint16_t)channel, opl3_instrument_defs[channel+9].CHAN_FRLO); //channels 9 to 17
		opl3_write(0x0100 | (uint16_t)OPL_CH_KBF_HI | (uint16_t)channel, opl3_instrument_defs[channel+9].CHAN_FNUM & 0xDF); //channels 9 to 17 remove key on
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

	//opl3_write((uint16_t)OPL_CH_FEED | (uint16_t)c,   opl3_instrument_defs[which].CHAN_FEED);
	
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
	

	/*
	textGotoXY(START_X_VIS   ,chan+START_HEIGHT_VIS_ALT);printf(" %02x ",inst.OP2_TVSKF);
	textGotoXY(START_X_VIS+4 ,chan+START_HEIGHT_VIS_ALT);printf(" %02x ",inst.OP1_TVSKF);
	textGotoXY(START_X_VIS+8 ,chan+START_HEIGHT_VIS_ALT);printf(" %02x ",inst.OP2_KSLVOL);
	textGotoXY(START_X_VIS+12,chan+START_HEIGHT_VIS_ALT);printf(" %02x ",inst.OP1_KSLVOL);
	textGotoXY(START_X_VIS+16,chan+START_HEIGHT_VIS_ALT);printf(" %02x ",inst.OP2_AD);
	textGotoXY(START_X_VIS+20,chan+START_HEIGHT_VIS_ALT);printf(" %02x ",inst.OP1_AD);
	textGotoXY(START_X_VIS+24,chan+START_HEIGHT_VIS_ALT);printf(" %02x ",inst.OP2_SR);
	textGotoXY(START_X_VIS+28,chan+START_HEIGHT_VIS_ALT);printf(" %02x ",inst.OP1_SR);
	textGotoXY(START_X_VIS+32,chan+START_HEIGHT_VIS_ALT);printf(" %02x ",inst.OP2_WAV);
	textGotoXY(START_X_VIS+36,chan+START_HEIGHT_VIS_ALT);printf(" %02x ",inst.OP1_WAV);
	*/
}


uint8_t opl3_set4OPS(uint8_t chan, bool steamRoll)
{
	uint8_t doIt = 0;
	uint8_t baseChan = 0; //for when pairs are made
	
	
	//opl3_write(OPL_PERC, chip_VT_PERC & 0xE0); //turn off all percussions
	
	if((chip_OPL3_PAIRS & 0x01) && (chan == 0 || chan == 3))
		{
			doIt = 0x01;
			baseChan = 0;
			printf("pairs 0-3 found");
		}
	else if((chip_OPL3_PAIRS & 0x02) && (chan == 1 || chan == 4))
		{
			doIt = 0x02;
			baseChan = 1;
			printf("pairs 1-4 found");
		}
	else if((chip_OPL3_PAIRS & 0x04)  && (chan == 2 || chan == 5))
		{
			doIt = 0x04;
			baseChan = 2;
			printf("pairs 2-5 found");
		}
	else if((chip_OPL3_PAIRS & 0x08) && (chan == 9 || chan == 12))
		{
			doIt = 0x08;
			baseChan = 9;
			printf("pairs 9-12 found");
		}
	else if((chip_OPL3_PAIRS & 0x10) && (chan == 10 || chan == 13))
		{
			doIt = 0x10;
			baseChan = 10;
			printf("pairs 10-13 found");
		}
	else if((chip_OPL3_PAIRS & 0x20) && (chan == 11 || chan == 14))
		{
			doIt = 0x20;
			baseChan = 11;
			printf("pairs 11-14 found");
		}
	else //we are in default 2-op instruments, set them all similar
		{
		opl3_write(OPL_FOE, 0x00);
		opl3_setInstrumentAllChannels(chan, steamRoll);
		resetClamps();
		}
	if(doIt > 0)
		{
		clampOP4();
		opl3_write(OPL_FOE, 0x3F); //max groupings
		opl3_setInstrument(opl3_instrument_defs[baseChan]   ,0);
		opl3_setInstrument(opl3_instrument_defs[baseChan+3] ,3);
		opl3_setInstrument(opl3_instrument_defs[baseChan]   ,1);
		opl3_setInstrument(opl3_instrument_defs[baseChan+3], 4);
		opl3_setInstrument(opl3_instrument_defs[baseChan]  , 2);
		opl3_setInstrument(opl3_instrument_defs[baseChan+3], 5);
	
		opl3_setInstrument(opl3_instrument_defs[baseChan]  , 9);
		opl3_setInstrument(opl3_instrument_defs[baseChan+3],12);
		opl3_setInstrument(opl3_instrument_defs[baseChan]  ,10);
		opl3_setInstrument(opl3_instrument_defs[baseChan+3],13);
		opl3_setInstrument(opl3_instrument_defs[baseChan]  ,11);
		opl3_setInstrument(opl3_instrument_defs[baseChan+3],14);
	
		}
	return doIt;
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



uint8_t opl3_shadow(uint8_t offset, uint8_t value, uint8_t hinb, bool iRT)
{
uint8_t chan=0;
uint8_t temp =0;
uint8_t isHitRelease = 0; //detects if there's a change in key on


if (hinb == 1) {
	chan = 9; // port 1 shift
	}

textSetColor(15,0);
switch(offset)
{
	case 0x01:
		chip_enable = value;
		break;
	case 0x04:
		if(hinb == 1) //opl3 only
			{
			//textGotoXY(45,2);printf("OP-4 pairings %02x", value);
			chip_OPL3_PAIRS = value; //keep track of OPL3 associations and mask unecessary bits that sometimes happen!
	
			if(iRT){textGotoXY(START_X_VIS+60,START_HEIGHT_VIS);printf(" %02x ",chip_OPL3_PAIRS);}
			}
		break;
	case 0x20 ... 0x22:
	case 0x28 ... 0x2A:
	case 0x30 ... 0x32:
		temp = offset-0x20;
		chan+=reverseChan(temp);
		opl3_instrument_defs[chan].OP1_TVSKF = value;
		if(iRT){textGotoXY(START_X_VIS,chan+START_HEIGHT_VIS);printf(" %02x ",value);}
		break;
	case 0x23 ... 0x25:
	case 0x2B ... 0x2D:
	case 0x33 ... 0x35:
		temp = offset-0x23;
		chan+=reverseChan(temp);
		opl3_instrument_defs[chan].OP2_TVSKF = value;
		if(iRT){textGotoXY(START_X_VIS+4,chan+START_HEIGHT_VIS);printf(" %02x ",value);}
		break;
	
	case 0x40 ... 0x42:
	case 0x48 ... 0x4A:
	case 0x50 ... 0x52:
		temp = offset-0x40;
		chan+=reverseChan(temp);
		opl3_instrument_defs[chan].OP1_KSLVOL = value;
		if(iRT){textGotoXY(START_X_VIS+8,chan+START_HEIGHT_VIS);printf(" %02x ",value);}
		break;
	case 0x43 ... 0x45:
	case 0x4B ... 0x4D:
	case 0x53 ... 0x55:
		temp = offset-0x43;
		chan+=reverseChan(temp);
		opl3_instrument_defs[chan].OP2_KSLVOL = value;
		if(iRT){textGotoXY(START_X_VIS+12,chan+START_HEIGHT_VIS);printf(" %02x ",value);}
		break;
	
	case 0x60 ... 0x62:
	case 0x68 ... 0x6A:
	case 0x70 ... 0x72:
		temp = offset-0x60;
		chan+=reverseChan(temp);
		opl3_instrument_defs[chan].OP1_AD = value;
		if(iRT){textGotoXY(START_X_VIS+16,chan+START_HEIGHT_VIS);printf(" %02x ",value);}
		break;
	case 0x63 ... 0x65:
	case 0x6B ... 0x6D:
	case 0x73 ... 0x75:
		temp = offset-0x63;
		chan+=reverseChan(temp);
		opl3_instrument_defs[chan].OP2_AD = value;
		if(iRT){textGotoXY(START_X_VIS+20,chan+START_HEIGHT_VIS);printf(" %02x ",value);}
		break;
	
	case 0x80 ... 0x82:
	case 0x88 ... 0x8A:
	case 0x90 ... 0x92:
		temp = offset-0x80;
		chan+=reverseChan(temp);
		opl3_instrument_defs[chan].OP1_SR = value;
		if(iRT){textGotoXY(START_X_VIS+24,chan+START_HEIGHT_VIS);printf(" %02x ",value);}
		break;
	case 0x83 ... 0x85:
	case 0x8B ... 0x8D:
	case 0x93 ... 0x95:
		temp = offset-0x83;
		chan+=reverseChan(temp);
		opl3_instrument_defs[chan].OP2_SR = value;
		if(iRT){textGotoXY(START_X_VIS+28,chan+START_HEIGHT_VIS);printf(" %02x ",value);}
		break;
	
	case 0xE0 ... 0xE2:
	case 0xE8 ... 0xEA:
	case 0xF0 ... 0xF2:
		temp = offset-0xE0;
		chan+=reverseChan(temp);
		opl3_instrument_defs[chan].OP1_WAV = value;
		if(iRT){textGotoXY(START_X_VIS+32,chan+START_HEIGHT_VIS);printf(" %02x ",value);}
		break;
	case 0xE3 ... 0xE5:
	case 0xEB ... 0xED:
	case 0xF3 ... 0xF5:
		temp = offset-0xE3;
		chan+=reverseChan(temp);
		opl3_instrument_defs[chan].OP2_WAV = value;
		if(iRT){textGotoXY(START_X_VIS+36,chan+START_HEIGHT_VIS);printf(" %02x ",value);}
		break;
	case 0xC0 ... 0xC8:
		chan += (offset - 0xC0);
		opl3_instrument_defs[chan].CHAN_FEED = value;
		if(iRT){textGotoXY(START_X_VIS+40,chan+START_HEIGHT_VIS);printf(" %02x ",value);}
		break;
	case 0xA0 ... 0xA8:
		chan += (offset - 0xA0);
		opl3_instrument_defs[chan].CHAN_FRLO = value;
		break;
	case 0xB0 ... 0xB8:
		chan += (offset - 0xB0);
		if((opl3_instrument_defs[chan].KEYHIT & 0x20) != (value & 0x20)) isHitRelease = 1;
		opl3_instrument_defs[chan].KEYHIT = (value & 0x20);
		opl3_instrument_defs[chan].CHAN_FNUM = value;
		if(iRT){textGotoXY(START_X_VIS+44,chan+START_HEIGHT_VIS);printf(" %02x ",value);}
		break;
	case 0xBD: //is a global thing, could repeat for all instrument?
		chip_VT_PERC = value;
		if(iRT){textGotoXY(START_X_VIS+48,chan+START_HEIGHT_VIS);printf(" %02x ",chip_VT_PERC);}
		break;
	case 0x08: //is a global thing, bit7 is composite sine wave no one understands and bit6 is noteselect split off for fnum messing about
		chip_NOTESEL = value;
		if(iRT){textGotoXY(START_X_VIS+56,chan+START_HEIGHT_VIS);printf(" %02x ",chip_NOTESEL);}
		break;
}

return isHitRelease;
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

