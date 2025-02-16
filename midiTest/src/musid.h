#ifndef MUSID_H
#define MUSID_H

#define SID1           0xD400
#define SID2           0xD500

//offsets from base address
#define SID_VOICE1    0x00
#define SID_VOICE2    0x07
#define SID_VOICE3    0x0E

#define SID_LO_B     0x00   //low byte frequency
#define SID_HI_B     0x01   //high byte frequency
#define SID_LO_PWDC  0x02   //low byte pulse wave duty cycle
#define SID_HI_PWDC  0x03   //high byte pulse wave duty cycle, only low nibble is used
#define SID_CTRL     0x04   //control register
#define SID_ATK_DEC  0x05   //hi-nibble = attack, lo-nibble = decay
#define SID_SUS_REL  0x06   //hi-nibble = sustain, lo-nibble = release duration
#define SID_LO_FCF   0x14   //filter cutoff freq low byte, only low nibble is used
#define SID_HI_FCF   0x15   //filter cutoff freq hi byte
#define SID_FRR      0x17   //filter resonance and routing
#define SID_FM_VC    0x18   //hi nibble = mode, lo nibble = volume

#include "f256lib.h"

void clearSIDRegisters(void);
void sidNoteOnOrOff(uint16_t voice, uint8_t ctrl, bool isOn);
void shutAllSIDVoices(void);


extern const uint8_t sidLow[];
extern const uint8_t sidHigh[];

#endif // MUSID_H