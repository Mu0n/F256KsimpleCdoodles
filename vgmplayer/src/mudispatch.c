#include "f256lib.h"
#include "../src/mudispatch.h"
#include "../src/muMidi.h"
#include "../src/muopl3.h"

//used for text UI to show chip activity in real time
uint8_t chipAct[5] ={0,0,0,0,0}; 

// The following is for polyphony
uint8_t polyOPL3Buffer[18] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
uint8_t polyOPL3ChanBits[18]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};


struct glTh *gPtr; 

void resetGlobals(struct glTh *gT)
{
	gT->opl3InstChoice = 0; //from 0 to whatever is inside the array for opl3
//lowest note on 88-key piano is a A more than 3 octaves below middle CLUT
//midi note number of that lowest note is dec=21

/*
typedef struct opl3Instrument {
	//OP2 Carrier, OP1 Modulator - they're in this order for easy copying from AdlibTracker 1.2 instrument information
	uint8_t OP2_TVSKF, OP1_TVSKF;
	uint8_t OP2_KSLVOL, OP1_KSLVOL;
	uint8_t OP2_AD, OP1_AD;
	uint8_t OP2_SR, OP1_SR;
	uint8_t OP2_WAV, OP1_WAV;
	uint8_t CHAN_FEED; //channel wide feed
   } opl3I;
   */
   

	gPtr->OPL3Values->OP2_TVSKF  = opl3_instrument_defs[1].OP2_TVSKF;
	gPtr->OPL3Values->OP1_TVSKF  = opl3_instrument_defs[1].OP1_TVSKF;
	gPtr->OPL3Values->OP2_KSLVOL = opl3_instrument_defs[1].OP2_KSLVOL;
	gPtr->OPL3Values->OP1_KSLVOL = opl3_instrument_defs[1].OP1_KSLVOL;
	gPtr->OPL3Values->OP2_AD     = opl3_instrument_defs[1].OP2_AD;
	gPtr->OPL3Values->OP1_AD     = opl3_instrument_defs[1].OP1_AD;
	gPtr->OPL3Values->OP2_SR     = opl3_instrument_defs[1].OP2_SR;
	gPtr->OPL3Values->OP1_SR     = opl3_instrument_defs[1].OP1_SR;
	gPtr->OPL3Values->CHAN_FEED  = opl3_instrument_defs[1].CHAN_FEED;
	
}

int8_t findFreeChannel(uint8_t *ptr, uint8_t howManyChans)
	{
	uint8_t i;
	for(i=0;i<howManyChans;i++)
		{
		if(ptr[i] == 0) return i;	
		}
	return -1;
	}
int8_t liberateChannel(uint8_t note, uint8_t *ptr, uint8_t howManyChans)
	{
	uint8_t i;
	for(i=0;i<howManyChans;i++)
		{
		if(ptr[i] == note) return i;	
		}
	return -1;
	}
	
	
//dispatchNote deals with all possibilities
void dispatchNote(bool isOn, uint8_t channel, uint8_t note, uint8_t speed, bool isBeat, uint8_t beatChan, struct glTh* gT){
	int8_t foundFreeChan=-1; //used when digging for a free channel for polyphony
	
	if(isOn && note ==0) return;

	if(isOn)
		{
			foundFreeChan = findFreeChannel(polyOPL3Buffer, 18);
			if(foundFreeChan != -1)
				{
				opl3_note(foundFreeChan, opl3_fnums[(note+5)%12], (note+5)/12-2, true);	
				polyOPL3Buffer[foundFreeChan] = note;
				}
			chipAct[4]++;
			}
	else 
		{
			foundFreeChan = liberateChannel(note, polyOPL3Buffer, 18);
			opl3_note(foundFreeChan, opl3_fnums[(note+5)%12], (note+5)/12-2, false);	
			polyOPL3Buffer[foundFreeChan] = 0;
			if(chipAct[4])chipAct[4]--;
			}
		return;
	

	
}
}