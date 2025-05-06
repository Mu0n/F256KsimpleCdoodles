//DEFINES
#define F256LIB_IMPLEMENTATION

#define ABS(a) ((a) < 0 ? -(a) : (a)) 

#define TIMER_CAR_COOKIE 0
#define TIMER_LANE_COOKIE 1
#define TIMER_CAR_FORCE_COOKIE 2
#define TIMER_CAR_DELAY 1
#define TIMER_LANE_DELAY 3
#define TIMER_CAR_FORCE_DELAY 3

#define TIMER_PLANET_DELAY 5
#define TIMER_PLANET_COOKIE 10

#define TIMER_SHIP_DELAY 2
#define TIMER_SHIP_COOKIE 11

#define TIMER_SHIP_MOVE_DELAY 3
#define TIMER_SHIP_MOVE_COOKIE 12

#define TIMER_SHOT_DELAY 1
#define TIMER_SHOT_COOKIE 20

#define VKY_SP0_CTRL  0xD900 //Sprite #0’s control register
#define VKY_SP0_AD_L  0xD901 // Sprite #0’s pixel data address register
#define VKY_SP0_AD_M     0xD902
#define VKY_SP0_AD_H     0xD903
#define VKY_SP0_POS_X_L  0xD904 // Sprite #0’s X position register
#define VKY_SP0_POS_X_H  0xD905
#define VKY_SP0_POS_Y_L  0xD906 // Sprite #0’s Y position register
#define VKY_SP0_POS_Y_H  0xD907
#define SPR_CLUT_COLORS       32
#define SPR_OFFSET 8

#define PSG_DEFAULT_VOL 0x4C

#define PAL_BASE    0x10000
#define BITMAP_BASE      0x10400  
#define PLANET_BASE      0x23000  
#define SHIP_BASE     	 0x24000
#define SHOT_BASE     	 0x24800        
  

//INCLUDES
#include "f256lib.h"
#include <stdlib.h>
#include "../src/muUtils.h" //contains helper functions I often use
#include "../src/muMidi.h"  //contains basic MIDI functions I often use
#include "../src/mupsg.h"   //contains PSG chip functions
#include "../src/muopl3.h"   //contains OPL3 chip functions
#include "../src/presetBeats.h"   //contains preset Beats
#include "../src/musid.h"   //contains SID chip functions
#include "../src/timerDefs.h"   //contains SID chip functions

EMBED(starspal, "../assets/stars.pal", 0x10000); //1kb
EMBED(starsbm, "../assets/stars.data",0x10400); //70kb
EMBED(planetBM, "../assets/planet.bin",0x23000); //4kb
EMBED(shipSP, "../assets/ship.bin",0x24000); //2kb
EMBED(shotBM, "../assets/shot.bin",0x24800); //64 bytes

//GLOBALS
struct timer_t planetScroll, shipExhaust, shotT, shipMT;

//GLOBALS
uint8_t chipAct[5] ={0,0,0,0,0}; //used for text UI to show chip activity in real time
uint8_t polyPSGBuffer[6] = {0,0,0,0,0,0};
uint8_t polyPSGChanBits[6]= {0x00,0x20,0x40,0x00,0x20,0x40};
uint8_t reservedPSG[6]={0,0,0,0,0,0};

uint8_t polySIDBuffer[6] = {0,0,0,0,0,0};
uint8_t sidChoiceToVoice[6] = {SID_VOICE1, SID_VOICE2, SID_VOICE3, SID_VOICE1, SID_VOICE2, SID_VOICE3};
uint8_t reservedSID[6] = {0,0,0,0,0,0};

uint8_t polyOPL3Buffer[9] = {0,0,0,0,0,0,0,0,0};
uint8_t polyOPL3ChanBits[9]={0,0,0,0,0,0,0,0,0};
uint8_t reservedOPL3[9]={0,0,0,0,0,0,0,0,0};	
//reference tempoLUT, close to 112.5 bpm where a quarter note = 32 frames = 0,533s
uint8_t tempoLUTRef[30] = {4, 8, 16, 32, 64, 128, //         32nd, 16th, 8th, 4th, half, whole
						   8,12, 24, 48, 96, 192, //dotted   32nd, 16th, 8th, 4th, half, whole
					      11,21, 32,  5, 11,  144,
						  0,0,0,0,0,0,
						  0,0,0,0,0,0}; //triplets of 4th: lengths of 1, 2, 3,  triplets of 8ths of length 1, 2, 3
						   
uint8_t mainTempoLUT[30]; //contains the delays between notes in prepared beats
uint8_t mainTempo=0;

bool needsToWaitExpiration = false;  //when tempo is changed, must wait to expire all pending timers before sounding again.
struct aB *theBeats;

uint8_t currentPal = 0;
	
const char *eraLabels[4] = {"1979 TI pulse generator   ","1983 SID chip          ","1988 Yamaha FM Synth      ","1991 General MIDI          "};

//FUNCTION PROTOTYPES
void setup(void);
void setPlanetPos(uint16_t, bool);
void prepTempoLUT(void);
void dispatchNote(bool, uint8_t, uint8_t, uint8_t, bool, uint8_t, bool, uint8_t);

void escReset()
{
	resetInstruments(true);		
	shutAllSIDVoices();
	shutPSG();
}
void emptyOPL3Buffer()
{
	uint8_t i;
	for(i=0;i<9;i++) polyOPL3Buffer[i]=0;
}
void emptySIDBuffer()
{
	uint8_t i;
	for(i=0;i<6;i++) polySIDBuffer[i]=0;
}
void emptyPSGBuffer()
{
	uint8_t i;
	for(i=0;i<6;i++) polyPSGBuffer[i]=0;
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
void launchBeat()
{
	uint8_t j, noteScoop=0, delayScoop=0;
	struct aT *theT;

	theT = malloc(sizeof(aTrack));
	
	if(theBeats[currentPal].isActive) needsToWaitExpiration = true; //orders a dying down of the beat
	if(needsToWaitExpiration) {
		for(j=0;j<theBeats[currentPal].howManyChans;j++)
			{
				getBeatTrackNoteInfo(theBeats, currentPal, j, &noteScoop, &delayScoop, theT);
				if(theT->chip == 1) {
					for(uint8_t i=0;i < 6;i++) if(reservedSID[i]==1) reservedSID[i]=0;
					
				}
				if(theT->chip == 2) {
					for(uint8_t i=0;i < 6;i++) if(reservedPSG[i]==1) reservedPSG[i]=0;
				}
				if(theT->chip == 3) {
					for(uint8_t i=0;i < 9;i++) if(reservedOPL3[i]==1) reservedOPL3[i]=0;
				}
			}
		free(theT);
		return; //we're not done finishing the beat, do nothing new
	}
	if(theBeats[currentPal].isActive == false) //starts the beat
		{
		theBeats[currentPal].isActive = true;
		mainTempo = theBeats[currentPal].suggTempo;
		
		prepTempoLUT();
		for(j=0;j<theBeats[currentPal].howManyChans;j++)
			{
				theBeats[currentPal].index[j] = 0;
				getBeatTrackNoteInfo(theBeats, currentPal, j, &noteScoop, &delayScoop, theT);
				
				beatSetInstruments(theT);
				dispatchNote(true, theT->chan, noteScoop,theT->chip==2?PSG_DEFAULT_VOL:0x7F, false, theT->chip, true, theT->inst);

				if(theT->chip == 1) reservedSID[theT->chan] = 1;
				theBeats[currentPal].activeCount+=1;
				theBeats[currentPal].timers[j].absolute = getTimerAbsolute(TIMER_FRAMES) + mainTempoLUT[delayScoop];
				setTimer(&(theBeats[currentPal].timers[j]));
			}
		}
		
	free(theT);
}	
//dispatchNote deals with all possibilities
void dispatchNote(bool isOn, uint8_t channel, uint8_t note, uint8_t speed, bool wantAlt, uint8_t whichChip, bool isBeat, uint8_t beatChan)
{
	uint16_t sidTarget, sidVoiceBase;
	int8_t foundFreeChan=-1; //used when digging for a free channel for polyphony
	
	if(isOn && note ==0) return;
	if(whichChip==0 || whichChip == 4) //MIDI
	{
		if(isOn) {
			midiNoteOn(channel, note, speed, false);
			chipAct[0]++;
		}
		else 
		{
			midiNoteOff(channel, note, speed, false);
			if(chipAct[0])chipAct[0]--;
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
				sidNoteOnOrOff(sidTarget + sidVoiceBase+SID_CTRL, fetchCtrl(beatChan), isOn);//if isBeat false, usually gPtr->sidInstChoice
				chipAct[2]++;
				polySIDBuffer[foundFreeChan] = note;					
				}
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
			sidNoteOnOrOff(sidTarget + sidVoiceBase+SID_CTRL, fetchCtrl(beatChan), isOn); //if isBeat false, usually gPtr->sidInstChoice
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
				chipAct[3]++;	
				}
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
			else foundFreeChan = findFreeChannel(polyOPL3Buffer, 9, reservedOPL3);
			if(foundFreeChan != -1)
				{
				opl3_note(foundFreeChan, opl3_fnums[(note+5)%12], (note+5)/12-2, true);	
				polyOPL3Buffer[foundFreeChan] = note;
				chipAct[4]++;
				}
			}
		else 
			{
			foundFreeChan = liberateChannel(note, polyOPL3Buffer, 9);
			opl3_note(foundFreeChan, opl3_fnums[(note+5)%12], (note+5)/12-2, false);	
			polyOPL3Buffer[foundFreeChan] = 0;
			if(chipAct[4])chipAct[4]--;
			}
	}
}	


void prepTempoLUT()
{
	uint8_t t;
	mainTempoLUT[0] = (uint8_t)(1.0f*112.5f*tempoLUTRef[0]/(1.0f*120));
	
	for(t=1;t<6;t++) {
		 mainTempoLUT[t] = mainTempoLUT[0];
		 for(uint8_t i=0;i<t;i++) mainTempoLUT[t] = mainTempoLUT[t]<<1;
	}
	for(t=6;t<12;t++) {
		 mainTempoLUT[t] = (mainTempoLUT[t-6]*2/3);
	}
	for(t=12;t<18;t++) {
		 mainTempoLUT[t] = (mainTempoLUT[t-12]*1/3);
	}
	for(t=18;t<24;t++) {
		 mainTempoLUT[t] = (mainTempoLUT[t-18]*4/3);
	}
	for(t=24;t<30;t++) {
		 mainTempoLUT[t] = (mainTempoLUT[t-24]*3);
	}
}



void setPlanetPos(uint16_t x, bool vis)
{
	spriteSetPosition(0, x   ,240);
	spriteSetPosition(1, x+32,240);
	spriteSetPosition(2, x+64,240);
	spriteSetPosition(3, x+96,240);
}

void setShipPos(uint16_t x, uint8_t y)
{
	spriteSetPosition(4, x   ,y);
}
void setup()
{
	uint32_t c,d,e,f;
	
	POKE(MMU_IO_CTRL, 0x00);
	// XXX GAMMA  SPRITE   TILE  | BITMAP  GRAPH  OVRLY  TEXT
	POKE(VKY_MSTR_CTRL_0, 0b00111111); //sprite,graph,overlay,text
	// XXX XXX  FON_SET FON_OVLY | MON_SLP DBL_Y  DBL_X  CLK_70
	POKE(VKY_MSTR_CTRL_1, 0b00000100); //font overlay, double height text, 320x240 at 60 Hz;
	POKE(VKY_LAYER_CTRL_0, 0b00000001); //bitmap 1 in layer 0, bitmap 0 in layer 1
	POKE(VKY_LAYER_CTRL_1, 0b00000010); //bitmap 2 in layer 2
	POKE(0xD00D,0x00); //force dark gray graphics background
	POKE(0xD00E,0x00);
	POKE(0xD00F,0x00);
	POKE(MMU_IO_CTRL,1);  //MMU I/O to page 1
	// Set up CLUT0.
	for(c=0;c<256;c++) 
	{	
		d=c+256;
		e=d+256;
		f=e+256;
		POKE((uint32_t)VKY_GR_CLUT_0+(uint32_t)c, FAR_PEEK((uint32_t)PAL_BASE+(uint32_t)c));
		POKE((uint32_t)VKY_GR_CLUT_0+(uint32_t)d, FAR_PEEK((uint32_t)PAL_BASE+(uint32_t)c));
		POKE((uint32_t)VKY_GR_CLUT_0+(uint32_t)e, FAR_PEEK((uint32_t)PAL_BASE+(uint32_t)c));
		POKE((uint32_t)VKY_GR_CLUT_0+(uint32_t)f, FAR_PEEK((uint32_t)PAL_BASE+(uint32_t)c));
	}
	
	for(c=0;c<256;c++) 
	{
		d=c+256;
		e=d+256;
		f=e+256;

		POKE((uint32_t)VKY_GR_CLUT_1+(uint32_t)c, FAR_PEEK((uint32_t)PAL_BASE+(uint32_t)d));
		POKE((uint32_t)VKY_GR_CLUT_1+(uint32_t)d, FAR_PEEK((uint32_t)PAL_BASE+(uint32_t)d));
		POKE((uint32_t)VKY_GR_CLUT_1+(uint32_t)e, FAR_PEEK((uint32_t)PAL_BASE+(uint32_t)d));
		POKE((uint32_t)VKY_GR_CLUT_1+(uint32_t)f, FAR_PEEK((uint32_t)PAL_BASE+(uint32_t)d));

	
	}
	for(c=0;c<256;c++) 
	{
		d=c+256;
		e=d+256;
		f=e+256;

		
		POKE((uint32_t)VKY_GR_CLUT_2+(uint32_t)c, FAR_PEEK((uint32_t)PAL_BASE+(uint32_t)e));
		POKE((uint32_t)VKY_GR_CLUT_2+(uint32_t)d, FAR_PEEK((uint32_t)PAL_BASE+(uint32_t)e));
		POKE((uint32_t)VKY_GR_CLUT_2+(uint32_t)e, FAR_PEEK((uint32_t)PAL_BASE+(uint32_t)e));
		POKE((uint32_t)VKY_GR_CLUT_2+(uint32_t)f, FAR_PEEK((uint32_t)PAL_BASE+(uint32_t)e));

	
	}
	for(c=0;c<256;c++) 
	{
		d=c+256;
		e=d+256;
		f=e+256;

		POKE((uint32_t)VKY_GR_CLUT_3+(uint32_t)c, FAR_PEEK((uint32_t)PAL_BASE+(uint32_t)f));
		POKE((uint32_t)VKY_GR_CLUT_3+(uint32_t)d, FAR_PEEK((uint32_t)PAL_BASE+(uint32_t)f));
		POKE((uint32_t)VKY_GR_CLUT_3+(uint32_t)e, FAR_PEEK((uint32_t)PAL_BASE+(uint32_t)f));
		POKE((uint32_t)VKY_GR_CLUT_3+(uint32_t)f, FAR_PEEK((uint32_t)PAL_BASE+(uint32_t)f));
	
	}
	
	POKE(MMU_IO_CTRL,0);
	
	//planet
	spriteDefine(0, PLANET_BASE      , 32, 0, 2);
	spriteDefine(1, PLANET_BASE+0x400, 32, 0, 2);
	spriteDefine(2, PLANET_BASE+0x800, 32, 0, 2);
	spriteDefine(3, PLANET_BASE+0xC00, 32, 0, 2);

	setPlanetPos(320,true);
	//ship
	spriteDefine(4, SHIP_BASE, 32, 0, 2);
	setShipPos(50,50);
	
	//shot
	spriteDefine(5, SHOT_BASE, 8, 0, 2);
	
	
	spriteSetVisible(0, true);
	spriteSetVisible(1, true);
	spriteSetVisible(2, true);
	spriteSetVisible(3, true);
	spriteSetVisible(4, true); //ship
	spriteSetVisible(5, false); //shot
	
	bitmapSetActive(2);
	bitmapSetAddress(2,BITMAP_BASE);
	bitmapSetCLUT(0);
	
	bitmapSetVisible(0,false);
	bitmapSetVisible(1,false);
	bitmapSetVisible(2,true); //furthermost to act as a real background
	
	
	planetScroll.cookie = TIMER_PLANET_COOKIE;
	planetScroll.units = TIMER_FRAMES;
	planetScroll.absolute = getTimerAbsolute(TIMER_FRAMES) + TIMER_PLANET_DELAY;
	setTimer(&planetScroll);
	
	shipExhaust.cookie = TIMER_SHIP_COOKIE;
	shipExhaust.units = TIMER_FRAMES;
	shipExhaust.absolute = getTimerAbsolute(TIMER_FRAMES) + TIMER_SHIP_DELAY;
	setTimer(&shipExhaust);
	
	shotT.cookie = TIMER_SHOT_COOKIE;
	shotT.units = TIMER_FRAMES;
	
	shipMT.cookie = TIMER_SHIP_MOVE_COOKIE;
	shipMT.absolute = getTimerAbsolute(TIMER_FRAMES) + TIMER_SHIP_MOVE_DELAY;
	setTimer(&shipMT);
	
	
		//Beats
	theBeats = malloc(sizeof(aBeat) * 4);
	setupBeats(theBeats);
	
	//super preparation
	escReset();
	
	//prep the tempoLUT
	prepTempoLUT();
	
	
	//Prep SID stuff
	clearSIDRegisters();
	prepSIDinstruments();
	setMonoSID();
	
	
	//Prep PSG stuff
	setMonoPSG();
	
	//Prep OPL3 stuff
	opl3_initialize();
	opl3_setInstrumentAllChannels(0);
	
				
}

void changePals(uint8_t pal)
{
	
	for(uint8_t c=0;c<5;c++)
	{
		POKE((uint32_t)VKY_SP0_CTRL + (uint32_t)SPR_OFFSET*(uint32_t)c, 0x11 | (pal<<1)); 
	}
	POKE((uint32_t)VKY_SP0_CTRL + (uint32_t)SPR_OFFSET*(uint32_t)5, 0x71 | (pal<<1)); 
	
	bitmapSetCLUT(pal);bitmapSetVisible(2,true);
}
	
int main(int argc, char *argv[]) {
	int16_t scrollPlanet=320;
	int16_t ship_X = 50;
	int16_t ship_Y = 50;
	int8_t ship_SX = 0;
	int8_t ship_SY = 0;
	
	uint8_t exhFlip = 0;
	int16_t shot_SX = 5;
	int16_t shot_X = 0;
	int16_t shot_Y = 0;
	uint16_t j=0;
	uint8_t noteScoop=0, delayScoop=0;
	struct aT *theT;
	
	theT = malloc(sizeof(aTrack));
	setup();	
	
	
				textGotoXY(0,0);printf("%s",eraLabels[currentPal]);	
	while(true)
		{
		kernelNextEvent();
		if(kernelEventData.type == kernelEvent(timer.EXPIRED))
			{	
			switch(kernelEventData.timer.cookie)
				{
					case TIMER_PLANET_COOKIE:
						scrollPlanet-=2;
						if(scrollPlanet <-64) scrollPlanet = 362;
						setPlanetPos(scrollPlanet, true);
						planetScroll.absolute = getTimerAbsolute(TIMER_FRAMES) + TIMER_PLANET_DELAY;
						setTimer(&planetScroll);
						break;
					case TIMER_SHIP_COOKIE:
						exhFlip++;
						if(exhFlip>1) exhFlip=0;
						spriteDefine(4,SHIP_BASE + (uint32_t)((uint16_t)exhFlip*0x400),32,currentPal,2);spriteSetVisible(4, true);
						shipExhaust.absolute = getTimerAbsolute(TIMER_FRAMES) + TIMER_SHIP_DELAY;
						setTimer(&shipExhaust);
						break;
					case TIMER_SHOT_COOKIE:
						shot_X += shot_SX;
						if(shot_X < 328)
						{
						spriteSetPosition(5,shot_X,shot_Y);
						shotT.absolute = getTimerAbsolute(TIMER_FRAMES) + TIMER_SHOT_DELAY;
						setTimer(&shotT);
						}
						else spriteSetVisible(5,false);
						break;
					case TIMER_SHIP_MOVE_COOKIE:
						if(ship_X < 320 && ship_X > 32) ship_X += (int16_t)ship_SX;
						if(ship_Y > 32 && ship_Y < 248) ship_Y += (int16_t)ship_SY;
						setShipPos(ship_X,ship_Y);
						shipMT.absolute = getTimerAbsolute(TIMER_FRAMES) + TIMER_SHIP_MOVE_DELAY;
						setTimer(&shipMT);
						break;
					case TIMER_BEAT ... 255:
					if(theBeats[currentPal].isActive)
						{
						switch(kernelEventData.timer.cookie)
							{
								default:
								j = kernelEventData.timer.cookie -TIMER_BEAT; //find the right track of the beat to deal with				
								break;
							}
							
						if (theBeats[currentPal].pending2x[j]== 2)theBeats[currentPal].pending2x[j]= 0; //restore for next pass
						
						if (theBeats[currentPal].pending2x[j]== 1){
							theBeats[currentPal].timers[j].absolute = getTimerAbsolute(TIMER_FRAMES) + mainTempoLUT[delayScoop];
							setTimer(&(theBeats[currentPal].timers[j]));
							theBeats[currentPal].pending2x[j]= 2;
							break;
							}	

	
						getBeatTrackNoteInfo(theBeats, currentPal, j, &noteScoop, &delayScoop, theT);								
						dispatchNote(false, theT->chan, noteScoop&0x7F,0x7F, false, theT->chip, true, theT->inst); // silence old note
						theBeats[currentPal].activeCount-=1;
	
								
						//check if we need to expire the beat and act to die things down
						if(needsToWaitExpiration)
							{
								
								if(theBeats[currentPal].activeCount > 0)
								{
									break;
								}
								else 
								{
									needsToWaitExpiration = false;
									theBeats[currentPal].isActive = false;
									if(theBeats[currentPal].pendingRelaunch) //oops, need to relaunch it at the end of it dying down
									{
										theBeats[currentPal].pendingRelaunch = false; //the relaunch will happen, so stop further relaunches for now
										launchBeat();
									}
									break;
								}
							}
							//otherwise proceed as normal and get the next note of that beat's track	
						else {
							
							theBeats[currentPal].index[j]++;	
							getBeatTrackNoteInfo(theBeats,currentPal, j, &noteScoop, &delayScoop, theT);
							if(theBeats[currentPal].index[j] ==  theT->count) 
								{
								theBeats[currentPal].index[j] = 0;
								getBeatTrackNoteInfo(theBeats, currentPal, j, &noteScoop, &delayScoop, theT);
								}
								
														
							if((noteScoop&0x80)==0x80) {
								if (theBeats[currentPal].pending2x[j]==0) theBeats[currentPal].pending2x[j]=1;
								}
								
							dispatchNote(true,  theT->chan, noteScoop&0x7F, theT->chip==2?PSG_DEFAULT_VOL:0x7F, false,  theT->chip, true, theT->inst);
							theBeats[currentPal].activeCount++;
							theBeats[currentPal].timers[j].absolute = getTimerAbsolute(TIMER_FRAMES) + mainTempoLUT[delayScoop];
							setTimer(&(theBeats[currentPal].timers[j]));
							}
						}
						
				}
			}
		else if(kernelEventData.type == kernelEvent(key.PRESSED))
			{
			switch(kernelEventData.key.raw)
				{	
				case 135: //F7
					launchBeat();
					break;
				case 32: //space
	
				if(theBeats[currentPal].isActive) needsToWaitExpiration = true; //orders a dying down of the beat
				if(needsToWaitExpiration) break; //we're not done finishing the beat, do nothing new
				

				currentPal++;
				if(currentPal>3) currentPal=0;
				changePals(currentPal);
				textGotoXY(0,0);printf("%s",eraLabels[currentPal]);


					
					break;
				case 97: //A (LEFT)
					ship_SX = -4;
					break;
				case 100: //D (RIGHT)
					ship_SX = 4;
					break;
				case 119: //W (UP)
					ship_SY = -2;
					break;
				case 115: //S (DOWN)
					ship_SY = 2;
					break;
				case 120: //X (STOP)
					ship_SY = ship_SX = 0;
					break;
				case 5: //ALT (SHOOT)
					shot_Y = ship_Y + 13;
					shot_X = ship_X + 28;
					spriteSetPosition(5,shot_X,shot_Y);
					spriteSetVisible(5,true);
					POKE((uint32_t)VKY_SP0_CTRL + (uint32_t)SPR_OFFSET*(uint32_t)5, 0x71 | (currentPal<<1)); 
					shotT.absolute = getTimerAbsolute(TIMER_FRAMES) + TIMER_SHOT_DELAY;
					setTimer(&shotT);
					break;
					
				default:
					textGotoXY(0,10);printf("%d  ",kernelEventData.key.raw);
				}
			}
			
		}
	return 0;}
