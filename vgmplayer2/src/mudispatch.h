#ifndef MUDISPATCH_H
#define MUDISPATCH_H

#include "f256lib.h"

typedef struct glTh
{
	bool wantVS1053;  
	uint8_t opl3InstChoice;
	//sid
	struct opl3Instrument *OPL3Values;
} globalThings;

int8_t findFreeChannel(uint8_t *, uint8_t);
int8_t liberateChannel(uint8_t , uint8_t *, uint8_t);
void dispatchNote(bool, uint8_t, uint8_t, uint8_t, bool, uint8_t, struct glTh*);
void clampOP4(void);
void resetClamps(void);

void resetGlobals(struct glTh*);
extern uint8_t chipAct[];

//extern uint8_t sidChoiceToVoice[];
extern uint8_t reservedSID[];

extern struct glTh *gPtr;
extern uint8_t chipAct[];
extern uint8_t selectableChan[];
#endif // MUDISPATCH_H
