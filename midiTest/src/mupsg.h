#ifndef MUPSG_H
#define MUPSG_H

#define PSG_SYS1  0xD6A1 //bit2 if 0, both mixed in mono. if 1, separate left and right speakers
#define PSG_LEFT  0xD600
#define PSG_RIGHT 0xD610
#define PSG_BOTH  0xD608

#define PSG_SILENCE 0x0F //use with bitwise OR with your attenuation command

#include "f256lib.h"

void shutPSG(void);
void psgNoteOn(uint8_t, uint16_t, uint8_t, uint8_t,uint8_t);
void psgNoteOff(uint8_t, uint16_t);
void setMonoPSG(void);
void setStereoPSG(void);
extern const uint8_t psgLow[];
extern const uint8_t psgHigh[];
#endif // MUPSG_H