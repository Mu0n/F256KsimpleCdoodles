#ifndef MUDISPATCH_H
#define MUDISPATCH_H

#include "f256lib.h"

typedef struct glTh
{
	bool wantVS1053;  
	uint8_t sidInstChoice;
	//sid
	struct sidInstrument *sidValues;
} globalThings;

int8_t findFreeChannel(uint8_t *, uint8_t);
int8_t liberateChannel(uint8_t , uint8_t *, uint8_t);
void dispatchNote(bool, uint8_t, uint8_t, uint8_t, bool, uint8_t, struct glTh*);

void resetGlobals(struct glTh*);
extern uint8_t chipAct[];

//extern uint8_t sidChoiceToVoice[];
extern uint8_t reservedSID[];

extern struct glTh *gPtr;
extern uint8_t chipAct[];

#endif // MUDISPATCH_H
