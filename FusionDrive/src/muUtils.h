#ifndef MUUTILS_H
#define MUUTILS_H

#define textColorGreen  0x04
#define textColorOrange 0x09
#define textColorRed    0x91
#define textColorBlue   0x07
#define textColorGray   0x05

#include "f256lib.h"


void wipeBitmapBackground(uint8_t, uint8_t, uint8_t);
void realTextClear(void);
bool setTimer(const struct timer_t *);
uint8_t getTimerAbsolute(uint8_t);
void injectChar40col(uint8_t, uint8_t, uint8_t, uint8_t);
void openAllCODEC(void);
void hitspace(void);

#endif // UTIL_H