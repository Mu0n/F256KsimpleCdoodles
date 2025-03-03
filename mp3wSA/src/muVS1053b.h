#ifndef MUVS1053b_H
#define MUVS1053b_H

//VS1053b serial bus
#define VS_SCI_CTRL  0xD700
//VS1053b CTRL modes
#define    CTRL_Start   0x01
#define    CTRL_RWn     0x02
#define    CTRL_Busy    0x08

#define VS_SCI_ADDR  0xD701
#define VS_SCI_DATA  0xD702   //2 bytes
#define VS_FIFO_STAT 0xD704   //2 bytes
#define VS_FIFO_DATA 0xD707

#include "f256lib.h"

void initVS1053Plugin(const uint16_t[], uint16_t);
void boostVSClock(void);
void initSpectrum(void);
void initRTMIDI(void);

#endif // MUVS1053b_H