#ifndef MUUTILS_H
#define MUUTILS_H

#include "f256lib.h"

void realTextClear(void);
bool setTimer(const struct timer_t *timer);
uint8_t getTimerAbsolute(uint8_t units);
void injectChar40col(uint8_t x, uint8_t y, uint8_t theChar, uint8_t col);
void openAllCODEC(void);

#endif // UTIL_H