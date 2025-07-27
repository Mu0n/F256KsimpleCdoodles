#include "D:\F256\llvm-mos\code\miditest\.builddir\trampoline.h"

#include "f256lib.h"
#include "../src/mudispatch.h"
#include "../src/muMidi.h"
#include "../src/musid.h"
#include "../src/muopl3.h"
#include "../src/mupsg.h"

//used for text UI to show chip activity in real time
uint8_t chipAct[5] ={0,0,0,0,0};

// The following is for polyphony for these 3 chips
uint8_t polySIDBuffer[6] = {0,0,0,0,0,0};
uint8_t sidChoiceToVoice[6] = {SID_VOICE1, SID_VOICE2, SID_VOICE3, SID_VOICE1, SID_VOICE2, SID_VOICE3};
uint8_t reservedSID[6] = {0,0,0,0,0,0}; //reserved array tells if a certain channel is being used by a ongoing beat and removes itself from being available for polyphony real time playing

uint8_t polyPSGBuffer[6] = {0,0,0,0,0,0};
uint8_t polyPSGChanBits[6]= {0x00,0x20,0x40,0x00,0x20,0x40};
uint8_t reservedPSG[6]={0,0,0,0,0,0};

uint8_t polyOPL3Buffer[18] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
uint8_t polyOPL3ChanBits[18]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
uint8_t reservedOPL3[18]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};


struct glTh *gPtr;

void resetGlobals(struct glTh *gT)
{
	gT->wantVS1053 = false;
	gT->sidInstChoice = 0; //from 0 to whatever is inside the array for sid
	gT->opl3InstChoice = 0; //from 0 to whatever is inside the array for opl3
//lowest note on 88-key piano is a A more than 3 octaves below middle CLUT
//midi note number of that lowest note is dec=21
    gT->chipChoice = 0; //0=MIDI, 1=SID, 2=PSG, 3=OPL3
	
	gT->chSelect = 0;
	gT->isTwinLinked = false;
	gT->selectBeat = 0;
	gT->mainTempo = 120;
	
	if(gT->prgInst == NULL) gT->prgInst = malloc(sizeof(uint8_t)*10);
	for(uint8_t i=0;i<10;i++) gT->prgInst[i] = 0; /* program change value, the MIDI instrument number, for chan 0, 1 and 9=percs */
	
}

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
void dispatchNote(bool isOn, uint8_t channel, uint8_t note, uint8_t speed, bool wantAlt, uint8_t whichChip, bool isBeat, uint8_t beatChan){
	uint16_t sidTarget, sidVoiceBase;
	int8_t foundFreeChan=-1; //used when digging for a free channel for polyphony
	
	if(isOn && note ==0) return;
	if(whichChip==0 || whichChip == 4) //MIDI
	{
		if(isOn) {
			midiNoteOn(channel, note, speed, whichChip==4?true:wantAlt);
			if(gPtr->isTwinLinked) midiNoteOn(1, note,speed, whichChip==4?true:wantAlt);
			if(wantAlt) chipAct[1]++;
			else chipAct[0]++;
		}
		else
		{
			midiNoteOff(channel, note, speed, whichChip==4?true:wantAlt);
		if(wantAlt) {if(chipAct[1])chipAct[1]--;}
		else {if(chipAct[0])chipAct[0]--;}
	
			if(gPtr->isTwinLinked)
			{
			midiNoteOff(1, note,speed, whichChip==4?true:wantAlt);
			if(wantAlt) {if(chipAct[1])chipAct[1]--;}
			else {if(chipAct[0])chipAct[0]--;}
			}
		}
		return;
	}
	if(whichChip==1) //SID
	{
		if(isOn)
		{
			if(isBeat) {
				foundFreeChan = channel; //if it's a preset, we already know which channel to target
				polySIDBuffer[channel]=note;
				}
			else foundFreeChan = findFreeChannel(polySIDBuffer, 6, reservedSID);
			if(foundFreeChan != -1)
				{
				sidTarget = foundFreeChan>2?SID2:SID1;
				sidVoiceBase = sidChoiceToVoice[foundFreeChan];
				POKE(sidTarget + sidVoiceBase + SID_LO_B, sidLow[note-11]); // SET FREQUENCY FOR NOTE 1
				POKE(sidTarget + sidVoiceBase + SID_HI_B, sidHigh[note-11]); // SET FREQUENCY FOR NOTE 1
				sidNoteOnOrOff(sidTarget + sidVoiceBase+SID_CTRL, isBeat?fetchCtrl(beatChan):fetchCtrl(gPtr->sidInstChoice), isOn);//if isBeat false, usually gPtr->sidInstChoice
				polySIDBuffer[foundFreeChan] = note;
				}
			chipAct[2]++;
		}
		else
		{
			if(isBeat) {
				foundFreeChan = channel; //if it's a preset, we already know which channel to target
				polySIDBuffer[channel]=0;
				}
			else foundFreeChan = liberateChannel(note, polySIDBuffer, 6);
			sidTarget = foundFreeChan>2?SID2:SID1;
			sidVoiceBase = sidChoiceToVoice[foundFreeChan];
			polySIDBuffer[foundFreeChan] = 0;
			POKE(sidTarget + sidVoiceBase+SID_LO_B, sidLow[note-11]); // SET FREQUENCY FOR NOTE 1
			POKE(sidTarget + sidVoiceBase+SID_HI_B, sidHigh[note-11]); // SET FREQUENCY FOR NOTE 1
				sidNoteOnOrOff(sidTarget + sidVoiceBase+SID_CTRL, isBeat?fetchCtrl(beatChan):fetchCtrl(gPtr->sidInstChoice), isOn);//if isBeat false, usually gPtr->sidInstChoice
			if(chipAct[2])chipAct[2]--;
		}
		return;
	}
	
	if(whichChip==2) //PSG
	{
		if(isOn) {
			if(isBeat) foundFreeChan = channel; //if it's a preset, we already know which channel to target
			else foundFreeChan = findFreeChannel(polyPSGBuffer, 6, reservedPSG);
			if(foundFreeChan != -1)
				{
				psgNoteOn(  polyPSGChanBits[foundFreeChan], //used to correctly address the channel in the PSG command
							foundFreeChan>2?PSG_RIGHT:PSG_LEFT, //used to send the command to the right left or right PSG
							psgLow[note-45],psgHigh[note-45],
							speed);
			    polyPSGBuffer[foundFreeChan] = note;
				}
			chipAct[3]++;
			}
		else
		{
			foundFreeChan = liberateChannel(note, polyPSGBuffer, 6);
			polyPSGBuffer[foundFreeChan] = 0;
			psgNoteOff(polyPSGChanBits[foundFreeChan], foundFreeChan>2?PSG_RIGHT:PSG_LEFT);
			if(chipAct[3])chipAct[3]--;
		}
		return;
	}
	
	
	if(whichChip==3) //OPL3
	{
		if(isOn)
			{
			if(isBeat) foundFreeChan = channel; //if it's a preset, we already know which channel to target
			else foundFreeChan = findFreeChannel(polyOPL3Buffer, 18, reservedOPL3);
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
	}
}
