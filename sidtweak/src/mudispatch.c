#include "f256lib.h"
#include "../src/mudispatch.h"
#include "../src/muMidi.h"
#include "../src/musid.h"

//used for text UI to show chip activity in real time
uint8_t chipAct[5] ={0,0,0,0,0}; 

// The following is for polyphony for these 3 chips
uint8_t polySIDBuffer[6] = {0,0,0,0,0,0};
uint8_t sidChoiceToVoice[6] = {SID_VOICE1, SID_VOICE2, SID_VOICE3, SID_VOICE1, SID_VOICE2, SID_VOICE3};


struct glTh *gPtr; 

void resetGlobals(struct glTh *gT)
{
	gT->sidInstChoice = 0; //from 0 to whatever is inside the array for sid
//lowest note on 88-key piano is a A more than 3 octaves below middle CLUT
//midi note number of that lowest note is dec=21
	gPtr->sidValues->maxVolume = sid_instrument_defs[1].maxVolume;
	gPtr->sidValues->pwdLo = sid_instrument_defs[1].pwdLo;
	gPtr->sidValues->pwdHi = sid_instrument_defs[1].pwdHi;
	gPtr->sidValues->ad = sid_instrument_defs[1].ad;
	gPtr->sidValues->sr = sid_instrument_defs[1].sr;
	gPtr->sidValues->ctrl = sid_instrument_defs[1].ctrl;
	gPtr->sidValues->fcfLo = sid_instrument_defs[1].fcfLo;
	gPtr->sidValues->fcfHi = sid_instrument_defs[1].fcfHi;
	gPtr->sidValues->frr = sid_instrument_defs[1].frr;
	
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
	uint16_t sidTarget, sidVoiceBase;
	int8_t foundFreeChan=-1; //used when digging for a free channel for polyphony
	
	if(isOn && note ==0) return;

	if(isOn)
		{
			foundFreeChan = findFreeChannel(polySIDBuffer, 6);
			if(foundFreeChan != -1)
				{
				sidTarget = foundFreeChan>2?SID2:SID1;
				sidVoiceBase = sidChoiceToVoice[foundFreeChan];
				POKE(sidTarget + sidVoiceBase + SID_LO_B, sidLow[note-11]); // SET FREQUENCY FOR NOTE 1 
				POKE(sidTarget + sidVoiceBase + SID_HI_B, sidHigh[note-11]); // SET FREQUENCY FOR NOTE 1 
				sidNoteOnOrOff(sidTarget + sidVoiceBase+SID_CTRL, gPtr->sidValues->ctrl, isOn);//if isBeat false, usually gPtr->sidInstChoice
				polySIDBuffer[foundFreeChan] = note;					
				}
				chipAct[2]++;
		}
	else 
		{
			foundFreeChan = liberateChannel(note, polySIDBuffer, 6);
			sidTarget = foundFreeChan>2?SID2:SID1;
			sidVoiceBase = sidChoiceToVoice[foundFreeChan];
			polySIDBuffer[foundFreeChan] = 0;
			POKE(sidTarget + sidVoiceBase+SID_LO_B, sidLow[note-11]); // SET FREQUENCY FOR NOTE 1 
			POKE(sidTarget + sidVoiceBase+SID_HI_B, sidHigh[note-11]); // SET FREQUENCY FOR NOTE 1 
			sidNoteOnOrOff(sidTarget + sidVoiceBase+SID_CTRL, gPtr->sidValues->ctrl, isOn); //if isBeat false, usually gPtr->sidInstChoice
			if(chipAct[2])chipAct[2]--;
			
		}
		return;
	

	
}
}