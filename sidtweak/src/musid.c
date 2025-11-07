#include "f256lib.h"
#include "../src/musid.h"
#include "../src/textUI.h"

const char *sid_instruments_names[] = {
	"Triangle",
	"SawTooth",
	"Pulse",
	"Noise",
	"Pad",
};

sidI sid_instrument_defs[] = {
	{.maxVolume=0x0F,.pwdLo=0x44,.pwdHi=0x00,.ad=0x27,.sr=0x5B,.ctrl=0x10,.fcfLo=0x00,.fcfHi=0x00,.frr=0x00},
	{.maxVolume=0x0F,.pwdLo=0xFF,.pwdHi=0x07,.ad=0x09,.sr=0x00,.ctrl=0x20,.fcfLo=0x00,.fcfHi=0x00,.frr=0x00},
	{.maxVolume=0x0F,.pwdLo=0x00,.pwdHi=0x08,.ad=0x00,.sr=0x40,.ctrl=0x40,.fcfLo=0x00,.fcfHi=0x00,.frr=0x00},
	{.maxVolume=0x0F,.pwdLo=0x44,.pwdHi=0x00,.ad=0x70,.sr=0x00,.ctrl=0x80,.fcfLo=0x00,.fcfHi=0x00,.frr=0x00},
	{.maxVolume=0x0A,.pwdLo=0x90,.pwdHi=0x04,.ad=0xD6,.sr=0xBA,.ctrl=0x40,.fcfLo=0x00,.fcfHi=0x00,.frr=0x00}
};


uint8_t fetchCtrl(uint8_t which)
{
	return sid_instrument_defs[which].ctrl;
}
const uint8_t sid_instrumentsSize = 5;

const uint8_t sidHigh[] = {
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02,
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x03, 0x03, 0x03, 0x03, 0x03, 0x04,
    0x04, 0x04, 0x04, 0x05, 0x05, 0x05, 0x06, 0x06, 0x06, 0x07, 0x07, 0x08,
    0x08, 0x08, 0x09, 0x0A, 0x0A, 0x0B, 0x0C, 0x0C, 0x0D, 0x0E, 0x0F, 0x10,
    0x10, 0x11, 0x13, 0x14, 0x15, 0x16, 0x18, 0x19, 0x1A, 0x1C, 0x1E, 0x20,
    0x21, 0x23, 0x26, 0x28, 0x2A, 0x2D, 0x30, 0x32, 0x35, 0x39, 0x3C, 0x40,
    0x43, 0x47, 0x4C, 0x50, 0x55, 0x5A, 0x60, 0x65, 0x6B, 0x72, 0x78, 0x80,
    0x87, 0x8F, 0x98, 0xA1, 0xAB, 0xB5, 0xC0, 0xCB, 0xD7, 0xE4, 0xF1, 0x00
};

const uint8_t sidLow[] = {
    0x0F, 0x1F, 0x30, 0x43, 0x56, 0x6A, 0x80, 0x96, 0xAF, 0xC8, 0xE3, 0x00,
    0x1F, 0x3F, 0x61, 0x86, 0xAC, 0xD5, 0x00, 0x2D, 0x5E, 0x91, 0xC7, 0x01,
    0x3E, 0x7F, 0xC3, 0x0C, 0x58, 0xAA, 0x00, 0x5B, 0xBC, 0x23, 0x8F, 0x02,
    0x7C, 0xFE, 0x86, 0x18, 0xB1, 0x54, 0x00, 0xB7, 0x78, 0x46, 0x1F, 0x05,
    0xF9, 0xFC, 0x0D, 0x30, 0x63, 0xA8, 0x01, 0x6E, 0xF1, 0x8C, 0x3F, 0x0B,
    0xF3, 0xF8, 0x1A, 0x60, 0xC6, 0x51, 0x02, 0xDD, 0xE3, 0x18, 0x7E, 0x16,
    0xE7, 0xF0, 0x35, 0xC0, 0x8D, 0xA3, 0x05, 0xBB, 0xC7, 0x30, 0xFD, 0x2D,
    0xCE, 0xE1, 0x6A, 0x80, 0x1A, 0x46, 0x0B, 0x77, 0x8F, 0x61, 0xFA, 0x5B
};


void clearSIDRegisters(void)
{
	uint8_t i;
	for(i=0;i<=0x18;i++) 
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

void sid_setInstrument(uint8_t sidChip, uint8_t voice, struct sidInstrument inst)
{ 
	uint16_t addrVoice = (sidChip==0?(uint16_t)SID1:(uint16_t)SID2);
	if(voice==1) addrVoice += (uint16_t)SID_VOICE2;
	if(voice==2) addrVoice += (uint16_t)SID_VOICE3;
	
	POKE(addrVoice+(uint16_t)SID_LO_PWDC, inst.pwdLo); // SET PULSE WAVE DUTY LOW BYTE
	POKE(addrVoice+(uint16_t)SID_HI_PWDC, inst.pwdHi); // SET PULSE WAVE DUTY HIGH BYTE
	POKE(addrVoice+(uint16_t)SID_ATK_DEC, inst.ad); // SET ATTACK;DECAY
	POKE(addrVoice+(uint16_t)SID_SUS_REL, inst.sr); // SET SUSTAIN;RELEASE
	POKE(addrVoice+(uint16_t)SID_CTRL, inst.ctrl); 	 // SET CTRL as triangle
}	

void sid_StageOne()
{ 
	sid_StageTwo(0,0);
	sid_StageTwo(0,1);
	sid_StageTwo(0,2);
	sid_StageTwo(1,0);
	sid_StageTwo(1,1);
	sid_StageTwo(1,2);
	
	POKE(SID1+SID_LO_FCF, sid_fields[2].value & 0x07);
	POKE(SID1+SID_HI_FCF, sid_fields[1].value | (sid_fields[0].value << 4));
	POKE(SID1+SID_FRR,    sid_fields[4].value | (sid_fields[3].value << 4));
	POKE(SID1+SID_FM_VC,  sid_fields[6].value | (sid_fields[5].value << 4));
	
	POKE(SID2+SID_LO_FCF, sid_fields[2].value & 0x07);
	POKE(SID2+SID_HI_FCF, sid_fields[1].value | (sid_fields[0].value << 4));
	POKE(SID2+SID_FRR,    sid_fields[4].value | (sid_fields[3].value << 4));
	POKE(SID2+SID_FM_VC,  sid_fields[6].value | (sid_fields[5].value << 4));
}	
	
void sid_StageTwo(uint8_t sidChip, uint8_t voice)
{
	uint16_t addrVoice = (sidChip==0?(uint16_t)SID1:(uint16_t)SID2);
	if(voice==1) addrVoice += (uint16_t)SID_VOICE2;
	if(voice==2) addrVoice += (uint16_t)SID_VOICE3;
	
	POKE(addrVoice+(uint16_t)SID_LO_PWDC, sid_fields[15].value | (sid_fields[14].value <<4)); // SET PULSE WAVE DUTY LOW BYTE
	POKE(addrVoice+(uint16_t)SID_HI_PWDC, sid_fields[13].value); // SET PULSE WAVE DUTY HIGH BYTE
	POKE(addrVoice+(uint16_t)SID_ATK_DEC, sid_fields[10].value | (sid_fields[9].value << 4)); // SET ATTACK;DECAY
	POKE(addrVoice+(uint16_t)SID_SUS_REL, sid_fields[12].value | (sid_fields[11].value << 4)); // SET SUSTAIN;RELEASE
	POKE(addrVoice+(uint16_t)SID_CTRL, (sid_fields[8].value & 0x0E) | (sid_fields[7].value << 4)); 	 // SET CTRL as triangle
}
void sid_setSIDWide(uint8_t which)
{
	POKE(SID1+SID_LO_FCF,sid_instrument_defs[which].fcfLo);
	POKE(SID1+SID_HI_FCF,sid_instrument_defs[which].fcfHi);
	POKE(SID1+SID_FRR, sid_instrument_defs[which].frr);
	POKE(SID1+SID_FM_VC, sid_instrument_defs[which].maxVolume);
	
	POKE(SID2+SID_LO_FCF,sid_instrument_defs[which].fcfLo);
	POKE(SID2+SID_HI_FCF,sid_instrument_defs[which].fcfHi);
	POKE(SID2+SID_FRR, sid_instrument_defs[which].frr);
	POKE(SID2+SID_FM_VC, sid_instrument_defs[which].maxVolume);
}

void sid_setInstrumentAllChannels(uint8_t which)
{
	uint8_t c;
	for(c=0;c<3;c++)
	{
	sid_setInstrument(0,c,sid_instrument_defs[which]);
	sid_setInstrument(1,c,sid_instrument_defs[which]);
	}
	sid_setSIDWide(which);
}

void prepSIDinstruments()
{
	sid_setInstrumentAllChannels(1);
}

void setMonoSID()
{
	uint8_t sys1;
	
	sys1=PEEK(SID_SYS1);
	sys1 = sys1 & 0b11110111;
	POKE(SID_SYS1,sys1);
}
void setStereoSID()
{
	uint8_t sys1;
	
	sys1=PEEK(SID_SYS1);
	sys1 = sys1 & 0b11111111;
	POKE(SID_SYS1,sys1);
}
}