#ifndef MUVS1053b_H
#define MUVS1053b_H
/////
//VS1053b serial bus
/////


#define VS_SCI_CTRL  0xD700
//VS1053b CTRL modes
#define    CTRL_Start   0x01 //1: start transfer, followed by 0 to stop
#define    CTRL_RWn     0x02 //1: read mode, 0: write mode
#define    CTRL_Busy    0x80 //if set, spi transfer is busy

#define VS_SCI_ADDR  0xD701
//VS1053b specific SCI addresses
#define VS_SCI_ADDR_MODE     0x00
#define VS_SCI_ADDR_STATUS   0x01
#define VS_SCI_ADDR_BASS     0x02
#define VS_SCI_ADDR_CLOCKF   0x03
#define VS_SCI_ADDR_WRAM     0x06
#define VS_SCI_ADDR_WRAMADDR 0x07
#define VS_SCI_ADDR_VOL      0x0B

#define VS_SCI_DATA  0xD702   //2 bytes
#define VS_FIFO_COUNT 0xD704   //2 bytes
#define VS_FIFO_DATA 0xD707

#include "f256lib.h"

void initVS1053Plugin(const uint16_t[], uint16_t);
void boostVSClock(void);
void initSpectrum(void);
void initRTMIDI(void);
void getCenterSAValues(uint16_t, uint16_t *);
void getCenterSABands(void);
uint16_t checkClock(void);
uint16_t getNbBands(void);

#endif // MUVS1053b_H