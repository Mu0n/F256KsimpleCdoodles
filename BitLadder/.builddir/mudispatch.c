#include "D:\F256\llvm-mos\code\BitLadder\.builddir\trampoline.h"

#include "f256lib.h"
#include "../src/mudispatch.h"
#include "../src/muMidi.h"
#include "../src/musid.h"
#include "../src/mupsg.h"
#include "../src/muUtils.h"

// The following is for polyphony for these 3 chips
uint8_t sidChoiceToVoice[6] = {SID_VOICE1, SID_VOICE2, SID_VOICE3, SID_VOICE1, SID_VOICE2, SID_VOICE3};
uint16_t psgChoiceToVoice[6] = {PSG_LEFT, PSG_LEFT, PSG_LEFT, PSG_RIGHT, PSG_RIGHT, PSG_RIGHT};
uint8_t polyPSGChanBits[6]= {0x00,0x20,0x40,0x00,0x20,0x40};

//dispatchNote deals with all possibilities
void dispatchNote(bool isOn, uint8_t channel, uint8_t note, uint8_t speed, bool wantAlt, uint8_t whichChip, bool isBeat, uint8_t beatChan){
	uint16_t sidTarget, sidVoiceBase;
	uint16_t psgTarget;
	uint8_t bareChip = (whichChip&0x0F); //just get the low nibble, which indicates which sound chip we want to target
	
	if(bareChip == 0x0F) return; //don't bother with muted mappings
	
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
				sidTarget = beatChan>2?SID2:SID1;
				sidVoiceBase = sidChoiceToVoice[beatChan];
				POKE(sidTarget + sidVoiceBase + SID_LO_B, sidLow[note-11]); // SET FREQUENCY FOR NOTE 1
				POKE(sidTarget + sidVoiceBase + SID_HI_B, sidHigh[note-11]); // SET FREQUENCY FOR NOTE 1
				sidNoteOnOrOff(sidTarget + sidVoiceBase + SID_CTRL, sidInstruments[beatChan].ctrl, isOn);//if isBeat false, usually gPtr->sidInstChoice

		return;
	}


	if(bareChip==2) //PSG
	{
			psgTarget = psgChoiceToVoice[beatChan];
			if(isOn) psgNoteOn(  polyPSGChanBits[beatChan], //used to correctly address the channel in the PSG command
						psgTarget, //used to send the command to the right left or right PSG
						psgLow[note-33],psgHigh[note-33],
						speed);
			else psgNoteOff(polyPSGChanBits[beatChan], psgTarget);
		return;
	}
	if(bareChip==3) //OPL3
	{
		return;
	}
	
	
}
