#ifndef MUPSG_H
#define MUPSG_H

#define PSG_ADDR 0xD608
#define PSG_SILENCE 0x9F

#include "f256lib.h"

void psgNoteOn(uint8_t loByte, uint8_t hiByte);
void psgNoteOff(void);

#endif // MUPSG_H