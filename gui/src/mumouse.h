#ifndef MUMOUSE_H
#define MUMOUSE_H

#define PS2_M_MODE_EN 0xD6E0
#define PS2_M_X_LO    0xD6E2
#define PS2_M_X_HI    0xD6E3
#define PS2_M_Y_LO    0xD6E4
#define PS2_M_Y_HI    0xD6E5

#define PS2_CTRL 0xD640
#define PS2_M_IN 0xD643
#define PS2_STAT 0xD644

#include "f256lib.h"

void prepMouse(void);
void showMouse();
void hideMouse();

#endif // MUMOUSE_H
