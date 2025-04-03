#include "f256lib.h"
#include "../src/globals.h"


void resetGlobals(struct glTh *gT)
{
	uint8_t i;

	gT->wantVS1053 = false;
	gT->prgInst = malloc(sizeof(uint8_t)*10);
    if(gT->prgInst == NULL) printf("\n error allocating");
	gT->sidInstChoice = 0; //from 0 to whatever is inside the array for sid
	gT->opl3InstChoice = 0; //from 0 to whatever is inside the array for opl3
	gT->chSelect = 0; //restricted to channel 0, 1 or 9 (for percs)
//lowest note on 88-key piano is a A more than 3 octaves below middle CLUT
//midi note number of that lowest note is dec=21
    gT->chipChoice = 0; //0=MIDI, 1=SID, 2=PSG, 3=OPL3
	gT->isTwinLinked = false; //when true, sends out midi notes to both channels when using a midi controller or space bar
    gT->selectBeat = 0; //selected beat preset from 0 to 3
	gT->mainTempo = 120;
	
	for(i=0;i<10;i++) gT->prgInst[i] = 0; /* program change value, the MIDI instrument number, for chan 0, 1 and 9=percs */
}
