#ifndef MUSID_H
#define MUSID_H

#define SID_SYS1  0xD6A1 //bit2 if 0, both mixed in mono. if 1, separate left and right speakers

//main base SID addresses
#define SID1           0xD400
#define SID2           0xD500

//offsets from base address
#define SID_VOICE1    0x00
#define SID_VOICE2    0x07
#define SID_VOICE3    0x0E

//offsets from each voice
#define SID_LO_B     0x00   //low byte frequency
#define SID_HI_B     0x01   //high byte frequency
#define SID_LO_PWDC  0x02   //low byte pulse wave duty cycle
#define SID_HI_PWDC  0x03   //high byte pulse wave duty cycle, only low nibble is used
#define SID_CTRL     0x04   //ctrl: Noise|Pulse|Saw|Tri | Test|Ring|Sync|Gate
#define SID_ATK_DEC  0x05   //hi-nibble = attack, lo-nibble = decay
#define SID_SUS_REL  0x06   //hi-nibble = sustain, lo-nibble = release duration
//offsets for each SID
#define SID_LO_FCF   0x14   //filter cutoff freq low byte
#define SID_HI_FCF   0x15   //filter cutoff freq hi byte
#define SID_FRR      0x17   //filter resonance and routing
#define SID_FM_VC    0x18   //hi nibble mode = 3ChOff|HPas|BPas|LPas, lo nibble = volume

#include "f256lib.h"

typedef struct sidInstrument {
    uint8_t maxVolume;      // hi nibble = mode, lo nibble = volume
	uint8_t pwdLo, pwdHi; // PULSE WAVE DUTY LOW BYTE and HIGH BYTE
	uint8_t ad, sr;         // ATTACK;DECAY ,  SUSTAIN;RELEASE
	uint8_t ctrl;           // CTRL 
	uint8_t fcfLo, fcfHi;   //filter cutoff freqs
	uint8_t frr;            //filter resonance & routing
   } sidI; 
		
		
void clearSIDRegisters(void);
void sidNoteOnOrOff(uint16_t, uint8_t, bool);
void shutAllSIDVoices(void);
void sid_setInstrument(uint8_t, uint8_t, struct sidInstrument);
void sid_setSIDWide(uint8_t);
void sid_setInstrumentAllChannels(uint8_t);
void prepSIDinstruments(void);
void setMonoSID(void);
void setStereoSID(void);
uint8_t fetchCtrl(uint8_t);


extern const char *sid_instruments_names[];
extern const uint8_t sid_instrumentsSize;
extern sidI sid_instrument_defs[];
extern const uint8_t sidLow[];
extern const uint8_t sidHigh[];

#endif // MUSID_H