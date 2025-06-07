#ifndef GLOBALS_H
#define GLOBALS_H

#include "f256lib.h"

//stash them all here for easy pass down to library functions, ie the text ui
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

void resetGlobals(struct glTh*);


#endif // GLOBALS_H