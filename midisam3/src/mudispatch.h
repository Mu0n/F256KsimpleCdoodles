#ifndef MUDISPATCH_H
#define MUDISPATCH_H

#include "f256lib.h"


int8_t findFreeChannel(uint8_t *, uint8_t, uint8_t *);
int8_t liberateChannel(uint8_t , uint8_t *, uint8_t);
void dispatchNote(bool, uint8_t, uint8_t, uint8_t, bool, uint8_t, bool);

extern uint8_t chipAct[];

//extern uint8_t sidChoiceToVoice[];
extern uint8_t reservedSID[];
extern uint8_t polySIDBuffer[];

extern uint8_t chipAct[];
extern uint8_t muteArray[];
extern uint8_t presentArray[];
extern uint8_t presentIndex;


#endif // MUDISPATCH_H
