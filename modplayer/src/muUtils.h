#ifndef MUUTILS_H
#define MUUTILS_H

#define textColorGreen  0x04
#define textColorOrange 0x09
#define textColorRed    0x91
#define textColorBlue   0x07
#define textColorGray   0x05

#include "f256lib.h"

uint8_t PEEK24(uint32_t addr);
uint8_t POKE24(uint32_t addr, uint8_t value);
void lilpause(uint8_t);
void wipeBitmapBackground(uint8_t, uint8_t, uint8_t);
bool setTimer(const struct timer_t *);
uint8_t getTimerAbsolute(uint8_t);
void hitspace(void);
bool isAnyK(void);
bool hasCaseLCD(void);
void shutDownIfNoK2(void);
bool isWave2(void);
bool isK2(void);

#endif // UTIL_H