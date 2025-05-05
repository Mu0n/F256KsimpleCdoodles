#ifndef MUDISPATCH_H
#define MUDISPATCH_H

#include "f256lib.h"

typedef struct glTh
{
	bool wantVS1053;  
	uint8_t *prgInst;
	uint8_t sidInstChoice;
	uint8_t opl3InstChoice;
	uint8_t chSelect;
	uint8_t chipChoice; 
	bool isTwinLinked;
	uint8_t selectBeat;
	uint8_t mainTempo;
} globalThings;

int8_t findFreeChannel(uint8_t *, uint8_t, uint8_t *);
int8_t liberateChannel(uint8_t , uint8_t *, uint8_t);
void dispatchNote(bool, uint8_t, uint8_t, uint8_t, bool, uint8_t, bool, uint8_t);

void resetGlobals(struct glTh*);
extern uint8_t chipAct[];
extern uint8_t sidChoiceToVoice[];
extern uint8_t reservedSID[];
extern struct glTh *gPtr;

#endif // MUDISPATCH_H
