#ifndef MUDISPATCH_H
#define MUDISPATCH_H

#include "f256lib.h"

typedef struct glTh
{
	bool wantVS1053;  
	uint8_t sidInstChoice;
	uint8_t opl3InstChoice;
	uint8_t chipChoice; 
	//sid
	struct sidInstrument *sidValues;
	//opl3 chip wide
	//opl3 channel wide
	//opl3 instrument wide
	uint8_t o_2_tvskf, o_1_tvskf;
	uint8_t o_2_kslvol, o_1_kslvol;
	uint8_t o_2_ad, o_1_ad;
	uint8_t o_2_sr, o_1_sr;
	uint8_t o_2_wav, o_1_wav;
	uint8_t o_chanfeed;
	
} globalThings;

int8_t findFreeChannel(uint8_t *, uint8_t, uint8_t *);
int8_t liberateChannel(uint8_t , uint8_t *, uint8_t);
void dispatchNote(bool, uint8_t, uint8_t, uint8_t, bool, uint8_t, bool, uint8_t);

void resetGlobals(struct glTh*);
extern uint8_t chipAct[];
/*
extern uint8_t sidChoiceToVoice[];
extern uint8_t reservedSID[];
extern uint8_t reservedPSG[];
extern uint8_t reservedOPL3[];
*/
extern struct glTh *gPtr;
extern uint8_t chipAct[];

#endif // MUDISPATCH_H
