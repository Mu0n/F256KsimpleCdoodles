#include "f256lib.h"
#include "../src/mudispatch.h"
#include "../src/muMidi.h"
#include "../src/musid.h"
#include "../src/muUtils.h"

//used for text UI to show chip activity in real time
uint8_t chipAct[5] ={0,0,0,0,0}; 

// The following is for polyphony for these 3 chips
uint8_t sidChoiceToVoice[6] = {SID_VOICE1, SID_VOICE2, SID_VOICE3, SID_VOICE1, SID_VOICE2, SID_VOICE3};

uint8_t polyPSGChanBits[6]= {0x00,0x20,0x40,0x00,0x20,0x40};
uint8_t muteArray[16]={0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0}; //0: not muted, 1: muted. has to be used by muTextUI and mudispatch
uint8_t presentArray[16]={0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0}; //0: not present, 1: present. has to be used by muTextUI
uint8_t presentIndex = 0; //for keeping up where we're at in the presentArray

int8_t findFreeChannel(uint8_t *ptr, uint8_t howManyChans, uint8_t *reserved)
	{
	uint8_t i;
	for(i=0;i<howManyChans;i++)
		{
		if(reserved[i]) continue;
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
void dispatchNote(bool isOn, uint8_t channel, uint8_t note, uint8_t speed, bool wantAlt, uint8_t whichChip, bool isBeat){
	uint16_t sidTarget, sidVoiceBase;
	//uint16_t psgTarget;
	uint8_t bareChip = (whichChip&0x0F); //just get the low nibble, which indicates which sound chip we want to target
	
	
	if(isOn && muteArray[channel]) return; //this channel is muted, don't do a note on
				
	if(isOn && note ==0) return;
	
	if(bareChip==0 || bareChip == 4) //MIDI
	{
		if(isOn) {
			midiNoteOn(channel, note, speed, bareChip==4?true:wantAlt);
		}
		else 
		{
			midiNoteOff(channel, note, speed, bareChip==4?true:wantAlt);
				
		}
		return;
	}
	
	if(bareChip==1) //SID
	{
				uint8_t subChan=(whichChip)>>4; //get the destination voice/channel of the target chip, use hi nibble only
				
				sidTarget = subChan>2?SID2:SID1;
				sidVoiceBase = sidChoiceToVoice[subChan];
				POKE(sidTarget + sidVoiceBase + SID_LO_B, sidLow[note-11]); // SET FREQUENCY FOR NOTE 1 
				POKE(sidTarget + sidVoiceBase + SID_HI_B, sidHigh[note-11]); // SET FREQUENCY FOR NOTE 1 
				sidNoteOnOrOff(sidTarget + sidVoiceBase + SID_CTRL, sidInstruments[subChan].ctrl, isOn);//if isBeat false, usually gPtr->sidInstChoice								

		return;
	}


	if(bareChip==2) //PSG
	{
			uint8_t subChan=(whichChip)>>4; //get the destination voice/channel of the target chip, use hi nibble only
			psgTarget = psgChoiceToVoice[subChan];
		

			if(isOn) psgNoteOn(  polyPSGChanBits[subChan], //used to correctly address the channel in the PSG command
						psgTarget, //used to send the command to the right left or right PSG
						psgLow[note-45],psgHigh[note-45],
						speed);


			else psgNoteOff(polyPSGChanBits[subChan], psgTarget);
		return;
	}
	if(bareChip==3) //OPL3
	{
		return;
	}
	
	
}
}