#ifndef MUMENU_H
#define MUMENU_H

#include "f256lib.h"


void displayMenu(uint8_t, uint8_t);
void displayOneItem(uint8_t, uint8_t, uint8_t);

extern char menuItems[][120];
extern uint8_t itemCount;

#endif // MUMENU_H