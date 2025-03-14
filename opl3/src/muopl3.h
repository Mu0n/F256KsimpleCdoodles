#ifndef MUOPL3_H
#define MUOPL3_H

#include "f256lib.h"

#define OPL_ADDR_L   0xD580   //Address pointer for ports 0x000 - 0x0FF
#define OPL_DATA     0xD581   //Data register for all ports
#define OPL_ADDR_H   0xD582   //Address pointer for ports 0x100 - 0x1FF

void opl3_initialize(void);
void opl3_write(uint16_t, uint8_t);
void opl3_note(uint8_t, uint16_t, uint8_t, bool);


extern const uint16_t opl3_fnums[12];

#endif // MUOPL3_H