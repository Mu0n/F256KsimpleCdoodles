#define F256LIB_IMPLEMENTATION

#define TIMER_FRAMES 0
#define TIMER_SECONDS 1

#define VELO_MIN 0x40 //minimum MIDI in note velocity, to avoid being too quiet

 //TIMER_TEXT for the 1-frame long text refresh timer for data display
 //TIMER_NOTE is for a midi note timer
#define TIMER_TEXT_COOKIE 0
#define TIMER_TEXT_DELAY 1

#include "../src/muVS1053b.h"  //contains basic MIDI functions I often use
#include "../src/muUtils.h" //contains helper functions I often use
#include "../src/muMidi.h"  //contains basic MIDI functions I often use
#include "../src/musid.h"   //contains SID chip functions
#include "../src/mupsg.h"   //contains PSG chip functions
#include "../src/muopl3.h"   //contains OPL3 chip functions
#include "../src/textui.h"   //has the text ui elements, specific to this project
#include "../src/globals.h"   //contains the globalThings structure
#include "../src/presetBeats.h"   //contains preset Beats

#define WITHOUT_TILE
#define WITHOUT_SPRITE
#include "f256lib.h"

EMBED(palpiano, "../assets/piano.pal", 0x30000);
EMBED(pia1, "../assets/piano.raw", 0x38000);

struct aB *theBeats;
struct glTh *gPtr;

struct timer_t spaceNotetimer, refTimer, snareTimer; //spaceNotetimer: used when you hit space, produces a 1s delay before NoteOff comes in
//refTimer: is 1 frame long, used to display updated text when you hit keys on a midi controller
//snareTimer is a pre-programmed beat at 30 frames

uint16_t note = 0x36, oldNote, oldCursorNote; /*note is the current midi hex note code to send. oldNote keeps the previous one so it can be Note_off'ed away after the timer expires, or a new note is called*/

//reference tempoLUT, close to 112.5 bpm where a quarter note = 32 frames = 0,533s
uint8_t tempoLUTRef[18] = {4, 8, 16, 32, 64, 128, //         32nd, 16th, 8th, 4th, half, whole
						   8,12, 24, 48, 96, 192, //dotted   32nd, 16th, 8th, 4th, half, whole
					      	11, 21, 32, 5, 11, 16}; //triplets of 4th: lengths of 1, 2, 3,  triplets of 8ths of length 1, 2, 3

uint8_t mainTempoLUT[18]; //contains the delays between notes in prepared beats

uint8_t diagBuffer[255]; //info dump on screen

uint8_t polyPSGBuffer[6] = {0,0,0,0,0,0};
uint8_t polyPSGChanBits[6]= {0x00,0x20,0x40,0x00,0x20,0x40};

uint8_t polySIDBuffer[6] = {0,0,0,0,0,0};
uint8_t sidChoiceToVoice[6] = {SID_VOICE1, SID_VOICE2, SID_VOICE3, SID_VOICE1, SID_VOICE2, SID_VOICE3};
uint8_t sidInstPerVoice[6] = {0x20, 0x20, 0x20, 0x20, 0x20, 0x20};

uint8_t polyOPL3Buffer[9] = {0,0,0,0,0,0,0,0,0};
uint8_t polyOPL3ChanBits[9]={0,0,0,0,0,0,0,0,0};

uint8_t *testArray;

bool instSelectMode = false; //is currently in the mode where you see the whole instrument list
bool needsToWaitExpiration = false;  //when tempo is changed, must wait to expire all pending timers before sounding again.
bool altHit = false, shiftHit = false; //keyboard modifiers

bool noteColors[88]={1,0,1, 1,0,1,0,1, 1,0,1,0,1,0,1, 1,0,1,0,1, 1,0,1,0,1,0,1, 1,0,1,0,1, 1,0,1,0,1,0,1, 1,0,1,0,1, 1,0,1,0,1,0,1, 1,0,1,0,1, 1,0,1,0,1,0,1, 1,0,1,0,1, 1,0,1,0,1,0,1, 1,0,1,0,1, 1,0,1,0,1,0,1, 1};



void escReset(void);


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
int8_t findFreeChannel(uint8_t *ptr, uint8_t howMany)
	{
	uint8_t i;
	for(i=0;i<howMany;i++)
		{
		if(ptr[i] == 0) return i;	
		}
	return -1;
	}
int8_t liberateChannel(uint8_t note, uint8_t *ptr, uint8_t howMany)
	{
	uint8_t i;
	for(i=0;i<howMany;i++)
		{
		if(ptr[i] == note) return i;	
		}
	return -1;
	}
	
//dispatchNote deals with all possibilities
void dispatchNote(bool isOn, uint8_t channel, uint8_t note, uint8_t speed, bool wantAlt)
{
	uint16_t sidTarget, sidVoiceBase;
	int8_t foundFreeChan=-1; //used when digging for a free channel for polyphony
	if(gPtr->chipChoice==0) //MIDI
	{
		if(isOn) {
			midiNoteOn(channel, note, speed, wantAlt);
			if(gPtr->isTwinLinked) midiNoteOn(1, note,speed, wantAlt);
		}
		else 
		{
			midiNoteOff(channel, note, speed, wantAlt);
			if(gPtr->isTwinLinked) midiNoteOff(1, note,speed, wantAlt);
		}
		return;
	}
	if(gPtr->chipChoice==1) //SID
	{
		if(isOn)
		{
			foundFreeChan = findFreeChannel(polySIDBuffer, 6);
			if(foundFreeChan != -1)
				{
				sidTarget = foundFreeChan>2?SID2:SID1;
				sidVoiceBase = sidChoiceToVoice[foundFreeChan];
				POKE(sidTarget + sidVoiceBase + SID_LO_B, sidLow[note-11]); // SET FREQUENCY FOR NOTE 1 
				POKE(sidTarget + sidVoiceBase + SID_HI_B, sidHigh[note-11]); // SET FREQUENCY FOR NOTE 1 
				sidNoteOnOrOff(sidTarget + sidVoiceBase+SID_CTRL, sidInstPerVoice[foundFreeChan], isOn);
				polySIDBuffer[foundFreeChan] = note;	
				}
		}
		else 
		{
			foundFreeChan = liberateChannel(note, polySIDBuffer, 6);
			sidTarget = foundFreeChan>2?SID2:SID1;
			sidVoiceBase = sidChoiceToVoice[foundFreeChan];
			polySIDBuffer[foundFreeChan] = 0;
			POKE(sidTarget + sidVoiceBase+SID_LO_B, sidLow[note-11]); // SET FREQUENCY FOR NOTE 1 
			POKE(sidTarget + sidVoiceBase+SID_HI_B, sidHigh[note-11]); // SET FREQUENCY FOR NOTE 1 
			sidNoteOnOrOff(sidTarget + sidVoiceBase+SID_CTRL, sidInstPerVoice[foundFreeChan], isOn);
		}
		return;
	}
	if(gPtr->chipChoice==2) //PSG
	{
		if(isOn) {
			foundFreeChan = findFreeChannel(polyPSGBuffer, 6);
			if(foundFreeChan != -1)
				{
				psgNoteOn(  polyPSGChanBits[foundFreeChan], //used to correctly address the channel in the PSG command
							foundFreeChan>2?PSG_RIGHT:PSG_LEFT, //used to send the command to the right left or right PSG
							psgLow[note-45],psgHigh[note-45],
							speed);
			    polyPSGBuffer[foundFreeChan] = note;

				}
			}
		else 
		{
			foundFreeChan = liberateChannel(note, polyPSGBuffer, 6);
			polyPSGBuffer[foundFreeChan] = 0;
			psgNoteOff(polyPSGChanBits[foundFreeChan], foundFreeChan>2?PSG_RIGHT:PSG_LEFT);
		}
		return;
	}
	if(gPtr->chipChoice==3) //OPL3
	{
		if(isOn) 
			{
			foundFreeChan = findFreeChannel(polyOPL3Buffer, 9);
			if(foundFreeChan != -1)
				{
				opl3_note(foundFreeChan, opl3_fnums[(note+5)%12], (note+5)/12-2, true);	
				polyOPL3Buffer[foundFreeChan] = note;
				}
			}
		else 
			{
			foundFreeChan = liberateChannel(note, polyOPL3Buffer, 9);
			opl3_note(foundFreeChan, opl3_fnums[(note+5)%12], (note+5)/12-2, false);	
			polyOPL3Buffer[foundFreeChan] = 0;
			}
	}
}	
void prepTempoLUT()
{
	uint8_t t;
	for(t=0;t<18;t++) {
		mainTempoLUT[t] = (uint8_t)(1.0f*112.5f*tempoLUTRef[t]/(1.0f*gPtr->mainTempo)); 
	}
	mainTempoLUT[12] = mainTempoLUT[14]-mainTempoLUT[13]; 
	//corrects rounding error and makes these triplets compatible with a quarter note
	
	mainTempoLUT[15] = mainTempoLUT[17]-mainTempoLUT[16]; 
	//same with triplet eights
}

//setup is called just once during initial launching of the program
void setup()
{
	uint16_t c;

	//Foenix Vicky stuff
	POKE(MMU_IO_CTRL, 0x00);
	// XXX GAMMA  SPRITE   TILE  | BITMAP  GRAPH  OVRLY  TEXT
	POKE(VKY_MSTR_CTRL_0, 0b00101111); //sprite,graph,overlay,text
	// XXX XXX  FON_SET FON_OVLY | MON_SLP DBL_Y  DBL_X  CLK_70
	POKE(VKY_MSTR_CTRL_1, 0b00010000); //font overlay, 320x240 at 60 Hz;
	POKE(VKY_LAYER_CTRL_0, 0b00010000); //bitmap 0 in layer 0, bitmap 1 in layer 1
	POKE(VKY_LAYER_CTRL_1, 0b00000010); //bitmap 2 in layer 2
	POKE(0xD00D,0x00); //force black graphics background
	POKE(0xD00E,0x00);
	POKE(0xD00F,0x00);
	
	POKE(MMU_IO_CTRL,1);  //MMU I/O to page 1
	//prep to copy over the palette to the CLUT
	for(c=0;c<1023;c++) 
	{
		POKE(VKY_GR_CLUT_0+c, FAR_PEEK(0x30000+c)); //palette for piano
	}
	
	POKE(MMU_IO_CTRL,0); //MMU I/O to page 0
	
	//piano bitmap at layer 0	
	bitmapSetActive(0);
	bitmapSetAddress(0,0x38000);
	bitmapSetVisible(0,true);
	bitmapSetCLUT(0);
	bitmapSetVisible(1,false);
	bitmapSetVisible(2,false);
	
	//Globals
	gPtr = malloc(sizeof(globalThings));
	
	//Beats
	theBeats = malloc(sizeof(aBeat) * 4);
	setupBeats(theBeats);
	
	//super preparation
	escReset();
	
	//prep the tempoLUT
	prepTempoLUT();
	
	//Prep all the kernel timers	
	refTimer.units = TIMER_FRAMES;
	refTimer.absolute = getTimerAbsolute(TIMER_FRAMES) + TIMER_TEXT_DELAY;
	refTimer.cookie = TIMER_TEXT_COOKIE;
	setTimer(&refTimer);


	textSetColor(textColorOrange,0x00);
	emptyMIDIINBuffer();
	
	//Prep SID stuff
	clearSIDRegisters();
	prepSIDinstruments();
	setMonoSID();
	
	//Prep PSG stuff
	setMonoPSG();
	
	//Prep OPL3 stuff
	opl3_initialize();
	opl3_setDefaultInstruments();

	for(c=0;c<255;c++) diagBuffer[c] = 0;
	
	//codec enable all lines	
	//openAllCODEC();
	if(isWave2()) 
	{
	//boost the VS1053b clock speed
		boostVSClock();
	//initialize the VS1053b real time midi plugin
	initRTMIDI();
	}
}
void escReset()
{
	resetInstruments(gPtr->wantVS1053);
	resetInstruments(~gPtr->wantVS1053);		
	POKE(gPtr->wantVS1053?MIDI_FIFO_ALT:MIDI_FIFO,0xC2);
	POKE(gPtr->wantVS1053?MIDI_FIFO_ALT:MIDI_FIFO,0x73);//woodblock
	resetGlobals(gPtr);						
	realTextClear();
	refreshInstrumentText(gPtr);
	textTitle(gPtr);
	shutAllSIDVoices();
	shutPSG();
	instSelectMode=false; //returns to default mode	
}

void shutAllMIDIchans()
{
midiShutAChannel(0, gPtr->wantVS1053);
midiShutAChannel(1, gPtr->wantVS1053);
midiShutAChannel(9, gPtr->wantVS1053);
}



void dealKeyPressed(uint8_t keyRaw)
{
	
	uint16_t i;
	uint8_t j;
					
	switch(keyRaw)
	{
		case 148: //enter
				if(instSelectMode){
					realTextClear();
					textTitle(gPtr);
					instSelectMode=false;
				}
				break;	
		case 146: // top left backspace, meant as reset
			escReset();
			break;
		case 129: //F1
			instSelectMode = !instSelectMode; //toggle this mode
			if(instSelectMode == true) instListShow(gPtr);
			else if(instSelectMode==false) {
				realTextClear();
				textTitle(gPtr);
				}
			break;
		case 131: //F3
			if(instSelectMode==false) {
				gPtr->isTwinLinked=false;
				gPtr->chSelect++;
				if(gPtr->chSelect == 2) 
					{
					gPtr->chSelect = 9;
					gPtr->isTwinLinked=false;
				}
				if(gPtr->chSelect == 10) gPtr->chSelect = 0;
				channelTextMenu(gPtr);
				refreshInstrumentText(gPtr);
				shutAllMIDIchans();
			}
			break;
		case 133: //F5
			if(instSelectMode==false) {
				
				if(theBeats[gPtr->selectBeat].isActive) needsToWaitExpiration = true; //orders a dying down of the beat
				if(needsToWaitExpiration) break; //we're not done finishing the beat, do nothing new
				
				if(theBeats[gPtr->selectBeat].isActive == false) //cycles the beat
					{
					gPtr->selectBeat+=1;
					if(gPtr->selectBeat==4)gPtr->selectBeat=0;
					refreshBeatTextChoice(gPtr);
					}
			}
			break;
		case 135: //F7
			if(instSelectMode==false) {
				
				if(theBeats[gPtr->selectBeat].isActive) needsToWaitExpiration = true; //orders a dying down of the beat
				if(needsToWaitExpiration) break; //we're not done finishing the beat, do nothing new
				
				if(theBeats[gPtr->selectBeat].isActive == false) //starts the beat
				{
					theBeats[gPtr->selectBeat].isActive = true;
					gPtr->mainTempo = theBeats[gPtr->selectBeat].suggTempo;
					prepTempoLUT();
					updateTempoText(gPtr->mainTempo);
					for(j=0;j<theBeats[gPtr->selectBeat].howMany;j++)
					{
						theBeats[gPtr->selectBeat].index[j] = 0;
						midiNoteOn(theBeats[gPtr->selectBeat].channel[j],  //channel
								   theBeats[gPtr->selectBeat].notes[j][0],    //note
								   0x7F,								//speed
								   gPtr->wantVS1053);							//midi chip selection    
						theBeats[gPtr->selectBeat].activeCount+=1;
						theBeats[gPtr->selectBeat].timers[j].absolute = getTimerAbsolute(TIMER_FRAMES) + 		mainTempoLUT[theBeats[gPtr->selectBeat].delays[j][0]];
						setTimer(&(theBeats[gPtr->selectBeat].timers[j]));
					}
				}
			}
			break;
		case 0xb6: //up arrow
			if(instSelectMode==false)
				{
					if(gPtr->chipChoice==0)
					{
						if(gPtr->prgInst[gPtr->chSelect] < 127 - shiftHit*9) gPtr->prgInst[gPtr->chSelect] = gPtr->prgInst[gPtr->chSelect] + 1 + shiftHit *9; //go up 10 instrument ticks if shift is on, otherwise just 1
						if(altHit) gPtr->prgInst[gPtr->chSelect] = 127; //go to highest instrument, 127
						prgChange(gPtr->prgInst[gPtr->chSelect],gPtr->chSelect, gPtr->wantVS1053);
						prgChange(gPtr->prgInst[gPtr->chSelect],gPtr->chSelect, !gPtr->wantVS1053);
					}
					if(gPtr->chipChoice==1)
					{
						if(gPtr->sidInstChoice<5) gPtr->sidInstChoice++;
					}
					refreshInstrumentText(gPtr);
				}
				else if(instSelectMode==true)
				{
					if(gPtr->chipChoice==0)
					{
						if(gPtr->prgInst[gPtr->chSelect] > (shiftHit?29:2))
						{
							modalMoveUp(gPtr, shiftHit);
							prgChange(gPtr->prgInst[gPtr->chSelect],gPtr->chSelect, gPtr->wantVS1053);
							prgChange(gPtr->prgInst[gPtr->chSelect],gPtr->chSelect, !gPtr->wantVS1053);
						}
					}
					if(gPtr->chipChoice==1 && gPtr->sidInstChoice>0) modalMoveUp(gPtr, shiftHit);
				}	
			break;
		case 0xb7: //down arrow
			if(instSelectMode==false)
					{
					if(gPtr->chipChoice==0)
						{
						if(gPtr->prgInst[gPtr->chSelect] > 0 + shiftHit*9) gPtr->prgInst[gPtr->chSelect] = gPtr->prgInst[gPtr->chSelect] - 1 - shiftHit *9; //go down 10 instrument ticks if shift is on, otherwise just 1
						if(altHit) gPtr->prgInst[gPtr->chSelect] = 0; //go to lowest instrument, 0
						prgChange(gPtr->prgInst[gPtr->chSelect],gPtr->chSelect, gPtr->wantVS1053);
						prgChange(gPtr->prgInst[gPtr->chSelect],gPtr->chSelect, !gPtr->wantVS1053);
						}
					if(gPtr->chipChoice==1)
						{
							if(gPtr->sidInstChoice>0) gPtr->sidInstChoice--;
						}
						refreshInstrumentText(gPtr);
					}
				else if(instSelectMode==true)
				{
					if(gPtr->chipChoice==0)
						{
							if(gPtr->prgInst[gPtr->chSelect] < (shiftHit?98:125))
							{
								modalMoveDown(gPtr, shiftHit);
								prgChange(gPtr->prgInst[gPtr->chSelect],gPtr->chSelect, gPtr->wantVS1053);
								prgChange(gPtr->prgInst[gPtr->chSelect],gPtr->chSelect, !gPtr->wantVS1053);
							}
						}
					if(gPtr->chipChoice==1 && gPtr->sidInstChoice<5) modalMoveDown(gPtr, shiftHit);
				}	
			break;
		case 0xb8: //left arrow
			if(instSelectMode==false)
					{
					if(note > (0 + shiftHit*11)) 
						{
						if(gPtr->chipChoice > 0) dispatchNote(false, 0, note+0x15, 0, false);
						note = note - 1 - shiftHit * 11;
						}
					if(altHit) note = 0; //go to the leftmost note
					graphicsDefineColor(0, oldCursorNote+0x61,0x00,0x00,0x00); //remove old cursor position
					graphicsDefineColor(0, note+0x61,0xFF,0x00,0x00); //set new cursor position
					oldCursorNote = note;
					}
			else if(instSelectMode==true)
				{
				if(gPtr->prgInst[gPtr->chSelect] > 0) modalMoveLeft(gPtr);
				prgChange(gPtr->prgInst[gPtr->chSelect],gPtr->chSelect, gPtr->wantVS1053);
				prgChange(gPtr->prgInst[gPtr->chSelect],gPtr->chSelect, !gPtr->wantVS1053);
				}
			break;
		case 0xb9: //right arrow
			if(instSelectMode==false)
				{
				if(note < 87 - shiftHit*11) 
					{
					if(gPtr->chipChoice > 0) dispatchNote(false, 0, note+0x15, 0, false);
					note = note + 1 + shiftHit * 11;
					}
				if(altHit) note = 87; //go to the rightmost note
										
				graphicsDefineColor(0, oldCursorNote+0x61,0x00,0x00,0x00);//remove old cursor position
				graphicsDefineColor(0, note+0x61,0xFF,0x00,0x00); //set new cursor position
				oldCursorNote = note;
				}
			else if(instSelectMode==true)
				{
				if(gPtr->prgInst[gPtr->chSelect] < 127) modalMoveRight(gPtr);
				prgChange(gPtr->prgInst[gPtr->chSelect],gPtr->chSelect, gPtr->wantVS1053);
				prgChange(gPtr->prgInst[gPtr->chSelect],gPtr->chSelect, !gPtr->wantVS1053);
				}
			break;
		case 32: //space
				//Send a Note
				if(gPtr->chipChoice !=3)
				{
				dispatchNote(true, 0x90 | gPtr->chSelect ,note+0x15,0x7F, gPtr->wantVS1053);	
				//keep track of that note so we can Note_Off it when needed
				oldNote = note+0x15; //make it possible to do the proper NoteOff when the timer expires
				}
				else
					{
					
					dispatchNote(true, 0,note+0x15,0,false);
					}

			break;
		case 5: //alt modifier
			altHit = true;
			break;
		case 1: //shift modifier
			shiftHit = true;
			break;
		case 91: // '['
			if(theBeats[gPtr->selectBeat].isActive) {
				gPtr->mainTempo -= (1 + shiftHit*9);
				prepTempoLUT();
				updateTempoText(gPtr->mainTempo);
				
				needsToWaitExpiration = true; //orders a dying down of the beat
				theBeats[gPtr->selectBeat].pendingRelaunch=true;
			}
			if(needsToWaitExpiration) break; //we're not done finishing the beat, do nothing new
			
			if(theBeats[gPtr->selectBeat].isActive == false) //changes the tempo
			{
				needsToWaitExpiration=true; //order an expiration of the beat
				break; //not done expiring a beat, don't start a new one
			}							
			//proceed to change tempo freely if one isn't playing at all
			else if(instSelectMode==false && gPtr->mainTempo > 30 + shiftHit*9) {
				gPtr->mainTempo -= (1 + shiftHit*9);
				prepTempoLUT();
				updateTempoText(gPtr->mainTempo);
				
				shutAllMIDIchans();
				for(j=0;j<theBeats[gPtr->selectBeat].howMany;j++) theBeats[gPtr->selectBeat].index[j]=0;
			}
			break;
		case 93: // ']'
			if(theBeats[gPtr->selectBeat].isActive) {
				gPtr->mainTempo += (1 + shiftHit*9);
				prepTempoLUT();
				updateTempoText(gPtr->mainTempo);
				needsToWaitExpiration = true; //orders a dying down of the beat
				theBeats[gPtr->selectBeat].pendingRelaunch=true;
			}
			if(needsToWaitExpiration) break; //we're not done finishing the beat, do nothing new
			
			if(theBeats[gPtr->selectBeat].isActive)
				{
					needsToWaitExpiration=true; //order an expiration of the beat
					break; //not done expiring a beat, don't start a new one
				}		
			else if(instSelectMode==false && gPtr->mainTempo < 255 - shiftHit*9) {
				gPtr->mainTempo += (1 + shiftHit*9);
				prepTempoLUT();
				updateTempoText(gPtr->mainTempo);
				
				shutAllMIDIchans();
				for(j=0;j<theBeats[gPtr->selectBeat].howMany;j++) theBeats[gPtr->selectBeat].index[j]=0;
			}
			break;
		case 120: // X - twin link mode
			if(instSelectMode==false) {
				gPtr->isTwinLinked = !gPtr->isTwinLinked;
				if(gPtr->isTwinLinked) {
					gPtr->chSelect=0;
					textSetColor(textColorOrange,0x00);
					textGotoXY(0,30);printf("%c",0xFA);
					textGotoXY(0,31);printf("%c",0xFA);
					textGotoXY(0,32);textPrint(" ");
				} else 
				{ 
					gPtr->chSelect=0;
					textSetColor(textColorOrange,0x00);
					textGotoXY(0,30);printf("%c",0xFA);
					textGotoXY(0,31);textPrint(" ");
					textGotoXY(0,32);textPrint(" ");
				}
			
				channelTextMenu(gPtr);
				refreshInstrumentText(gPtr);
				shutAllMIDIchans();
			}
			break;
		case 109: // M - toggle the MIDI chip between default SAM2695 to VS1053b
			
			shutAllMIDIchans();
			gPtr->wantVS1053 = ~(gPtr->wantVS1053);
			shutAllMIDIchans();
			
			showMIDIChoiceText(gPtr);
			break;
		case 99: // C - chip select mode: 0=MIDI, 1=SID, (todo) 2= PSG, (todo) 3=OPL3
			gPtr->chipChoice+=1;
			if(gPtr->chipChoice==1) prepSIDinstruments(); //just arrived in sid, prep sid
			if(gPtr->chipChoice==2) 
			{
				clearSIDRegisters();
				emptySIDBuffer();
			}
			if(gPtr->chipChoice==3) 
			{
				emptyPSGBuffer();
				shutPSG();
			}
			if(gPtr->chipChoice>3)
			{
				opl3_quietAll();
				emptyOPL3Buffer();
				gPtr->chipChoice=0; //loop back to midi at the start of the cyle
			}
			showChipChoiceText(gPtr);
			textGotoXY(0,3);textPrint("                                        ");
			break;
		case 100: // D - diagnostics info dump
			printf("\nbuffer Dump=");
			for(i=0;i<12;i++)
			{
				for(j=0;j<80;j++) textPrint(" ");
			}
			textGotoXY(0,5);
			for(j=0;j<255;j++)
				{
				printf("%02x ",diagBuffer[j]);
				}
				printf(" hit space to go on");
				hitspace();
			break;
		case 101: // E - empty buffer for diagnostics info dump
			
			for(j=0;j<255;j++)
				{
				diagBuffer[j]=0;
				}
				printf("emptied buffer");
			break;
	}
	//the following line can be used to get keyboard codes
	//printf("\n %d",kernelEventData.key.raw);
}

void dealKeyReleased(uint8_t rawKey)
{
switch(rawKey)
	{
	case 5:  //alt modifier
		altHit = false;
		break;
	case 1: //shift modifier
		shiftHit = false;
		break;
	case 32: //space
		dispatchNote(false, gPtr->chSelect ,note+0x15,0x3F, gPtr->wantVS1053);						
		break;
	}
}

#define SEGMENT_MAIN
int main(int argc, char *argv[]) {
	uint16_t toDo;
	uint16_t i;
	uint8_t j;
	uint8_t recByte, detectedNote, detectedColor, lastCmd=0x90;
	bool nextIsNote = false; //detect a 0x9? or 0x8? command, the next is a note byte, used for coloring the keyboard note-rects
	bool nextIsSpeed = false; //detects if we're at the end of a note on or off trio of bytes
	bool isHit = false; // true is hit, false is released
	POKE(1,0);
	uint8_t storedNote; //stored values when twinlinked
	uint8_t lastNote = 0; // for monophonic chips, store this to mute last note before doing a new one
	uint8_t bufferIndex=0;
	bool opl3Active = false;
	bool psgActive = false;
	bool sidActive = false;

	setup();
	
	note=39;
	oldCursorNote=39;
	graphicsDefineColor(0, note+0x61,0xFF,0x00,0x00); 
	

	
	while(true) 
        {
		if(!(PEEK(MIDI_CTRL) & 0x02)) //rx not empty
			{
				toDo = PEEKW(MIDI_RXD) & 0x0FFF; //discard top 4 bits of MIDI_RXD+1
				
				//erase the region where the last midi bytes received are shown
				if(instSelectMode==false){
					textGotoXY(5,4);textPrint("                                                                                ");textGotoXY(5,4);
				}
					
				//deal with the MIDI bytes and exhaust the FIFO buffer
				for(i=0; i<toDo; i++)
				{	
				//get the next MIDI in FIFO buffer byte
				recByte=PEEK(MIDI_FIFO);
				if(recByte == 0xfe) continue; //active sense, ignored
				if(bufferIndex==255) bufferIndex = 0;
				diagBuffer[bufferIndex++] = recByte;
				
				if(nextIsSpeed) //this block activates when a note is getting finished on the 3rd byte ie 0x90 0x39 0x40 (noteOn middleC midSpeed)
					{
					nextIsSpeed = false;
					//force a minimum level with this instead: recByte<0x70?0x70:recByte 
					if(isHit == true) //turn the last one off before dealing with the new one
						{
						switch(gPtr->chipChoice) //remove last note before making new one
							{
							case 1:
								if(sidActive) 
									{
									dispatchNote(false, gPtr->chSelect,lastNote,recByte<VELO_MIN?VELO_MIN:recByte, gPtr->wantVS1053);
									}
								break;
							case 2:
								if(psgActive) 
									{
									dispatchNote(false, gPtr->chSelect,lastNote,recByte<VELO_MIN?VELO_MIN:recByte, gPtr->wantVS1053);
									}
								break;
							case 3:
								if(opl3Active) 
									{
									dispatchNote(false, gPtr->chSelect,lastNote,recByte<VELO_MIN?VELO_MIN:recByte, gPtr->wantVS1053);
									}
								break;
							}
						}
					dispatchNote(isHit, gPtr->chSelect,storedNote,recByte<VELO_MIN?VELO_MIN:recByte, gPtr->wantVS1053); //do the note or turn off the note
					if(isHit == false) //turn 'em off if the note is ended
						{
						switch(gPtr->chipChoice)
						{
							case 1:
								sidActive = false;
								break;
							case 2:
								psgActive = false;
								break;
							case 3:
								opl3Active = false;
								break;
							}
						}
					lastNote = storedNote;
					}
				else if(nextIsNote) //this block triggers if the previous byte was a NoteOn or NoteOff (0x90,0x80) command previously
					{
					//figure out which note on the graphic is going to be highlighted
					detectedNote = recByte-0x14;
					
					//first case is when the last command is a 0x90 'NoteOn' command
					if(isHit) graphicsDefineColor(0, detectedNote,0xFF,0x00,0xFF); //paint it as a hit note
					//otherwise it's a 0x80 'NoteOff' command
					else {
						detectedColor = noteColors[detectedNote-1]?0xFF:0x00;
						graphicsDefineColor(0, detectedNote,detectedColor,detectedColor,detectedColor); //swap back the original color according to this look up ref table noteColors
					}
					nextIsNote = false; //return to previous state after a note is being dealt with
					storedNote = recByte;
					nextIsSpeed = true;
					}
				else if((nextIsNote == false) && (nextIsSpeed == false)) //what command are we getting next?
					{
					switch(recByte & 0xF0)
						{
						case 0x90: //we know it's a 'NoteOn', get ready to analyze the note byte, which is next
							nextIsNote = true;
							isHit=true;
							lastCmd = recByte;
							break;
						case 0x80: //we know it's a 'NoteOff', get ready to analyze the note byte, which is next
							nextIsNote = true;
							isHit=false;
							lastCmd = recByte;
							break;
						case 0x00 ... 0x7F:
							storedNote = recByte;
							nextIsNote = false; //false because we just received it!
							nextIsSpeed = true;
							switch(lastCmd & 0xF0)
								{
								case 0x90:
									isHit=true;
									break;
								case 0x80:
									isHit=true;
									break;
								}
							break;
						}
					}

				//else if(gPtr->chipChoice==0 && nextIsNote == false && nextIsSpeed == false) POKE(gPtr->wantVS1053?MIDI_FIFO_ALT:MIDI_FIFO, recByte); //all other bytes are sent normally	

					
				}
			}
		
		kernelNextEvent();
        if(kernelEventData.type == kernelEvent(timer.EXPIRED))
            {
			switch(kernelEventData.timer.cookie)
				{
				//all user interface related to text update through a 1 frame timer is managed here
				case TIMER_TEXT_COOKIE:
					refTimer.absolute = getTimerAbsolute(TIMER_FRAMES) + TIMER_TEXT_DELAY;
					setTimer(&refTimer); 
					break;
				case  TIMER_BEAT_0:
					if(theBeats[gPtr->selectBeat].isActive)
					{
						midiNoteOff(theBeats[gPtr->selectBeat].channel[0], //channel
								    theBeats[gPtr->selectBeat].notes[0][theBeats[gPtr->selectBeat].index[0]], //note
									0x7F, //speed
									gPtr->wantVS1053 //midi chip selection
						);
						theBeats[gPtr->selectBeat].activeCount-=1;
						//check if we need to expire the beat and act to die things down
						if(needsToWaitExpiration)
							{
								if(theBeats[gPtr->selectBeat].activeCount > 0)
								{
									break;
								}
								else 
								{
									needsToWaitExpiration = false;
									theBeats[gPtr->selectBeat].isActive = false;
									if(theBeats[gPtr->selectBeat].pendingRelaunch) //oops, need to relaunch it at the end of it dying down
									{
										theBeats[gPtr->selectBeat].pendingRelaunch = false; //the relaunch will happen, so stop further relaunches for now
										theBeats[gPtr->selectBeat].isActive = true;
										//mainTempo = theBeats[selectBeat].suggTempo;
										prepTempoLUT();
										updateTempoText(gPtr->mainTempo);
										for(j=0;j<theBeats[gPtr->selectBeat].howMany;j++)
										{
											theBeats[gPtr->selectBeat].index[j] = 0;
											midiNoteOn(theBeats[gPtr->selectBeat].channel[j],  //channel
													   theBeats[gPtr->selectBeat].notes[j][0],    //note
													   0x7F,                         //speed
														gPtr->wantVS1053); //midi chip selection);								    
											theBeats[gPtr->selectBeat].activeCount+=1;
											theBeats[gPtr->selectBeat].timers[j].absolute = getTimerAbsolute(TIMER_FRAMES) + 		mainTempoLUT[theBeats[gPtr->selectBeat].delays[j][0]];
											setTimer(&(theBeats[gPtr->selectBeat].timers[j]));
										}
									}
									break;
								}
							}
						
						//otherwise proceed as normal and get the next note of that beat's track
						theBeats[gPtr->selectBeat].index[0]+=1;
						
						if(theBeats[gPtr->selectBeat].index[0] == theBeats[gPtr->selectBeat].noteCount[0]) theBeats[gPtr->selectBeat].index[0] = 0;
						
						midiNoteOn(theBeats[gPtr->selectBeat].channel[0], //channel
								    theBeats[gPtr->selectBeat].notes[0][theBeats[gPtr->selectBeat].index[0]], //note
									0x7F, //speed
									gPtr->wantVS1053 //midi chip selection
						);
						theBeats[gPtr->selectBeat].activeCount+=1;
						theBeats[gPtr->selectBeat].timers[0].absolute = getTimerAbsolute(TIMER_FRAMES) + mainTempoLUT[
																							theBeats[gPtr->selectBeat].delays[0][theBeats[gPtr->selectBeat].index[0]]
																						];
																												
						setTimer(&(theBeats[gPtr->selectBeat].timers[0]));
					}
					break;
				case TIMER_BEAT_1A ... TIMER_BEAT_3B:
					if(theBeats[gPtr->selectBeat].isActive)
					{
						switch(kernelEventData.timer.cookie)
						{
						case TIMER_BEAT_1A:
						case TIMER_BEAT_1B:
							j = kernelEventData.timer.cookie -TIMER_BEAT_1A; //find the right track of the beat to deal with				
							break;
						case TIMER_BEAT_2A:
						case TIMER_BEAT_2B:
							j = kernelEventData.timer.cookie -TIMER_BEAT_2A;
							break;
						case TIMER_BEAT_3A:
						case TIMER_BEAT_3B:
							j = kernelEventData.timer.cookie -TIMER_BEAT_3A;
							break;
						}
						
						midiNoteOff(theBeats[gPtr->selectBeat].channel[j], //channel
								    theBeats[gPtr->selectBeat].notes[j][theBeats[gPtr->selectBeat].index[j]], //note
									0x7F, //speed
									gPtr->wantVS1053); //midi chip selection
						theBeats[gPtr->selectBeat].activeCount-=1;
						//check if we need to expire the beat and act to die things down
						if(needsToWaitExpiration)
							{
								if(theBeats[gPtr->selectBeat].activeCount > 0)
								{
									break;
								}
								else 
								{
									needsToWaitExpiration = false;
									theBeats[gPtr->selectBeat].isActive = false;
									if(theBeats[gPtr->selectBeat].pendingRelaunch) //oops, need to relaunch it at the end of it dying down
									{
										theBeats[gPtr->selectBeat].pendingRelaunch = false; //the relaunch will happen, so stop further relaunches for now
										theBeats[gPtr->selectBeat].isActive = true;
										//mainTempo = theBeats[selectBeat].suggTempo;
										prepTempoLUT();
										updateTempoText(gPtr->mainTempo);
										for(j=0;j<theBeats[gPtr->selectBeat].howMany;j++)
										{
											theBeats[gPtr->selectBeat].index[j] = 0;
											midiNoteOn(theBeats[gPtr->selectBeat].channel[j],  //channel
													   theBeats[gPtr->selectBeat].notes[j][0],    //note
													   0x7F,								//speed
													   gPtr->wantVS1053);							//midi chip selection	    
											theBeats[gPtr->selectBeat].activeCount+=1;
											theBeats[gPtr->selectBeat].timers[j].absolute = getTimerAbsolute(TIMER_FRAMES) + 		mainTempoLUT[theBeats[gPtr->selectBeat].delays[j][0]];
											setTimer(&(theBeats[gPtr->selectBeat].timers[j]));
										}
									}
									break;
								}
							}
						//otherwise proceed as normal and get the next note of that beat's track	
						theBeats[gPtr->selectBeat].index[j]+=1;
						if(theBeats[gPtr->selectBeat].index[j] == theBeats[gPtr->selectBeat].noteCount[j]) theBeats[gPtr->selectBeat].index[j] = 0;
						midiNoteOn(theBeats[gPtr->selectBeat].channel[j], //channel
								    theBeats[gPtr->selectBeat].notes[j][theBeats[gPtr->selectBeat].index[j]], //note
									0x7F, //speed
									gPtr->wantVS1053); //midi chip selection
						theBeats[gPtr->selectBeat].activeCount+=1;
						theBeats[gPtr->selectBeat].timers[j].absolute = getTimerAbsolute(TIMER_FRAMES) + mainTempoLUT[theBeats[gPtr->selectBeat].delays[j][theBeats[gPtr->selectBeat].index[j]]];
						setTimer(&(theBeats[gPtr->selectBeat].timers[j]));
						}
					break;
				}	
            }
			
		else if(kernelEventData.type == kernelEvent(key.PRESSED))
			{
			dealKeyPressed(kernelEventData.key.raw);
			}
		else if(kernelEventData.type == kernelEvent(key.RELEASED))
			{
			dealKeyReleased(kernelEventData.key.raw);
			}		
		}
return 0;}
}