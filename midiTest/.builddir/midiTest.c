#include "D:\F256\llvm-mos\code\midiTest\.builddir\trampoline.h"

#define F256LIB_IMPLEMENTATION

#define TIMER_FRAMES 0
#define TIMER_SECONDS 1

#define VELO_MIN 0x40 //minimum MIDI in note velocity, to avoid being too quiet

 //TIMER_TEXT for the 1-frame long text refresh timer for data display
 //TIMER_NOTE is for a midi note timer
#define TIMER_TEXT_COOKIE 0
#define TIMER_TEXT_DELAY 9

#include "../src/muVS1053b.h"  //contains basic MIDI functions I often use
#include "../src/muMidi.h"  //contains basic MIDI functions I often use
#include "../src/mupsg.h"   //contains PSG chip functions
#include "../src/muopl3.h"   //contains OPL3 chip functions
#include "../src/presetBeats.h"   //contains preset Beats
#include "../src/beatFileOps.h"   //can save or restore beats from files
#include "../src/mudispatch.h"   //sends to various chips

#include "../src/musid.h"   //contains SID chip functions
#include "../src/textui.h"   //has the text ui elements, specific to this project
#include "../src/muUtils.h" //contains helper functions I often use
#include "stdio.h"

#define PSG_DEFAULT_VOL 0x4C

#define WITHOUT_TILE
#define WITHOUT_SPRITE
#include "f256lib.h"

EMBED(palpiano, "../assets/piano.pal", 0x30000);
EMBED(pia1, "../assets/piano.raw", 0x38000);

struct aB *theBeats;

struct timer_t spaceNotetimer, refTimer, snareTimer; //spaceNotetimer: used when you hit space, produces a 1s delay before NoteOff comes in
//refTimer: is 1 frame long, used to display updated text when you hit keys on a midi controller
//snareTimer is a pre-programmed beat at 30 frames

uint16_t note = 0x36, oldNote, oldCursorNote; /*note is the current midi hex note code to send. oldNote keeps the previous one so it can be Note_off'ed away after the timer expires, or a new note is called*/

//reference tempoLUT, close to 112.5 bpm where a quarter note = 32 frames = 0,533s
uint8_t tempoLUTRef[18] = {4, 8, 16, 32, 64, 128, //         32nd, 16th, 8th, 4th, half, whole
						   8,12, 24, 48, 96, 192, //dotted   32nd, 16th, 8th, 4th, half, whole
					      11,21, 32,  5, 11,  144}; //triplets of 4th: lengths of 1, 2, 3,  triplets of 8ths of length 1, 2, 3
	

uint8_t mainTempoLUT[18]; //contains the delays between notes in prepared beats

uint8_t diagBuffer[255]; //info dump on screen

uint8_t blockCharacters[] = {22,21,19,20,22,23,26,27,28,29,24,23,24,25,25,22,23,24,21,20,21,23,20};
uint8_t *testArray;

bool instSelectMode = false; //is currently in the mode where you see the whole instrument list
bool needsToWaitExpiration = false;  //when tempo is changed, must wait to expire all pending timers before sounding again.
bool altHit = false, shiftHit = false; //keyboard modifiers

bool noteColors[88]={1,0,1, 1,0,1,0,1, 1,0,1,0,1,0,1, 1,0,1,0,1, 1,0,1,0,1,0,1, 1,0,1,0,1, 1,0,1,0,1,0,1, 1,0,1,0,1, 1,0,1,0,1,0,1, 1,0,1,0,1, 1,0,1,0,1,0,1, 1,0,1,0,1, 1,0,1,0,1,0,1, 1,0,1,0,1, 1,0,1,0,1,0,1, 1};
uint8_t KeyCodesToMIDINotes[123] = {0,0,0,0,0,0,0,0,  //0-7
									0,0,0,0,0,0,0,0,  //8-15
									0,0,0,0,0,0,0,0,  //16-23
									0,0,0,0,0,0,0,0,  //24-31
									0,0,0,0,0,0,0,0,  //32-39
									0,0,0,0,55,0,57,59,  //40-47
									75,0,61,63,0,66,68,70,  //48-55
									0,73,0,58,0,78,0,0,  //56-63
									0,0,0,0,0,0,0,0,  //64-71
									0,0,0,0,0,0,0,0,  //72-79
									0,0,0,0,0,0,0,0,  //80-87
									0,0,0,77,0,79,0,0,  //88-95
									0,42,50,47,46,64,0,49,  //96-103
									51,72,0,54,56,53,52,74,  //104-111
									76,60,65,44,67,71,48,62,  //112-119
									45,69,43}; //120-122
	
	
void escReset(void);


void emptyOPL3Buffer()
{
	uint8_t i;
	for(i=0;i<18;i++) polyOPL3Buffer[i]=0;
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


void prepTempoLUT()
{
	uint8_t t;
	mainTempoLUT[0] = (uint8_t)(1.0f*112.5f*tempoLUTRef[0]/(1.0f*gPtr->mainTempo));
	
	for(t=1;t<6;t++) {
		 mainTempoLUT[t] = mainTempoLUT[0];
		 for(uint8_t i=0;i<t;i++) mainTempoLUT[t] = mainTempoLUT[t]<<1;
	}
	for(t=6;t<12;t++) {
		 mainTempoLUT[t] = (mainTempoLUT[t-6]*2/3);
	}
	mainTempoLUT[17] = mainTempoLUT[1] * 18;
	mainTempoLUT[11] = mainTempoLUT[17] /3 * 2;
	/*
	textGotoXY(0,10);
	for(t=0;t<18;t++) printf("%02d ",mainTempoLUT[t]);
	printf("\n");
	
	for(t=0;t<18;t++) printf("%02d ",t);
	*/
	//same with triplet eights
}


//setup is called just once during initial launching of the program
void setup()
{
	uint16_t c;

	//Foenix Vicky stuff
	POKE(MMU_IO_CTRL, 0x00);
	// XXX GAMMA  SPRITE   TILE  | BITMAP  GRAPH  OVRLY  TEXT
	POKE(VKY_MSTR_CTRL_0, 0b00001111); //sprite,graph,overlay,text
	// XXX XXX  FON_SET FON_OVLY | MON_SLP DBL_Y  DBL_X  CLK_70
	POKE(VKY_MSTR_CTRL_1, 0b00000000); //font overlay, 320x240 at 60 Hz;
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
	POKE(0xD840, 0xFF);  //blue
	POKE(0xD841, 0xFF); //green
	POKE(0xD842, 0xFF); //red
	POKE(0xD843, 0);
	
	//piano bitmap at layer 0
	bitmapSetActive(0);
	bitmapSetAddress(0,0x38000);
	bitmapSetVisible(0,true);
	bitmapSetCLUT(0);
	bitmapSetVisible(1,false);
	bitmapSetVisible(2,false);
	
	//Globals
	gPtr = malloc(sizeof(globalThings));
	resetGlobals(gPtr);
	
	//Beats
	theBeats = malloc(sizeof(aBeat) * presetBeatCount);
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
	opl3_setInstrumentAllChannels(0);

	for(c=0;c<255;c++) diagBuffer[c] = 0;
	
	boostVSClock();
	initRTMIDI();
	openAllCODEC();
	//codec enable all lines
/*
	if(isWave2())
	{
	//boost the VS1053b clock speed
	boostVSClock();
	//initialize the VS1053b real time midi plugin
	initRTMIDI();
	openAllCODEC();
	}
	*/
	
}
void escReset()
{
	resetInstruments(gPtr->wantVS1053);
	resetInstruments(~gPtr->wantVS1053);
	resetGlobals(gPtr);
	realTextClear();
	refreshInstrumentText(gPtr);
	textTitle(gPtr);
	shutAllSIDVoices();
	shutPSG();
	instSelectMode=false; //returns to default mode
	POKE(MIDI_FIFO_ALT,0xC2);
	POKE(MIDI_FIFO_ALT,0x73);//woodblock
	POKE(MIDI_FIFO,0xC2);
	POKE(MIDI_FIFO,0x73);//woodblock
	gPtr->prgInst[0]=0;gPtr->prgInst[1]=0;gPtr->prgInst[9]=0;
	
}


void shutAllMIDIchans()
{
	midiShutAChannel(0, true);midiShutAChannel(0, false);
	midiShutAChannel(1, true);midiShutAChannel(1, false);
	midiShutAChannel(9, true);midiShutAChannel(9, false);
}

void launchBeat()
{
	uint8_t j, noteScoop=0, delayScoop=0;
	struct aT *theT;

	theT = malloc(sizeof(aTrack));
	
	if(theBeats[gPtr->selectBeat].isActive) needsToWaitExpiration = true; //orders a dying down of the beat
	if(needsToWaitExpiration) {
		for(j=0;j<theBeats[gPtr->selectBeat].howManyChans;j++)
			{
				getBeatTrackNoteInfo(theBeats, gPtr->selectBeat, j, &noteScoop, &delayScoop, theT);
				if(theT->chip == 1) {
					for(uint8_t i=0;i < 6;i++) if(reservedSID[i]==1) reservedSID[i]=0;
	
				}
				if(theT->chip == 2) {
					for(uint8_t i=0;i < 6;i++) if(reservedPSG[i]==1) reservedPSG[i]=0;
				}
				if(theT->chip == 3) {
					for(uint8_t i=0;i < 18;i++) if(reservedOPL3[i]==1) reservedOPL3[i]=0;
				}
			}
		free(theT);
		return; //we're not done finishing the beat, do nothing new
	}
	if(theBeats[gPtr->selectBeat].isActive == false) //starts the beat
		{
		theBeats[gPtr->selectBeat].isActive = true;
		gPtr->mainTempo = theBeats[gPtr->selectBeat].suggTempo;
	
		prepTempoLUT();
		updateTempoText(gPtr->mainTempo);
		for(j=0;j<theBeats[gPtr->selectBeat].howManyChans;j++)
			{
				theBeats[gPtr->selectBeat].index[j] = 0;
				getBeatTrackNoteInfo(theBeats, gPtr->selectBeat, j, &noteScoop, &delayScoop, theT);
	
				beatSetInstruments(theT);
				dispatchNote(true, theT->chan, noteScoop,theT->chip==2?PSG_DEFAULT_VOL:0x7F, gPtr->wantVS1053, theT->chip, true, theT->inst);

				if(theT->chip == 1) reservedSID[theT->chan] = 1;
				theBeats[gPtr->selectBeat].activeCount+=1;
				theBeats[gPtr->selectBeat].timers[j].absolute = getTimerAbsolute(TIMER_FRAMES) + mainTempoLUT[delayScoop];
				setTimer(&(theBeats[gPtr->selectBeat].timers[j]));
			}
		}
	
	free(theT);
}
void dealKeyPressed(uint8_t keyRaw)
{
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
					gPtr->selectBeat++;
					if((gPtr->selectBeat)==presetBeatCount)gPtr->selectBeat=0;
					refreshBeatTextChoice(gPtr);
					}
			}
			break;
		case 135: //F7
			if(instSelectMode==false) {
	
					launchBeat();
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
						if(gPtr->sidInstChoice<sid_instrumentsSize) gPtr->sidInstChoice++;
					}
					if(gPtr->chipChoice==3) if(gPtr->opl3InstChoice<opl3_instrumentsSize) gPtr->opl3InstChoice++;
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
					if(gPtr->chipChoice==1 && gPtr->sidInstChoice>0)
						{
						modalMoveUp(gPtr, shiftHit);
						sid_setInstrumentAllChannels(gPtr->sidInstChoice);
						}
					if(gPtr->chipChoice==3 && gPtr->opl3InstChoice>0)
					{
						modalMoveUp(gPtr, shiftHit);
						opl3_setInstrumentAllChannels(gPtr->opl3InstChoice);
					}
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
					if(gPtr->chipChoice==1) if(gPtr->sidInstChoice>0) gPtr->sidInstChoice--;
						refreshInstrumentText(gPtr);
					}
				else if(instSelectMode==true)
				{
					if(gPtr->chipChoice==0) //MIDI
						{
							if(gPtr->prgInst[gPtr->chSelect] < (shiftHit?98:125))
							{
								modalMoveDown(gPtr, shiftHit);
								prgChange(gPtr->prgInst[gPtr->chSelect],gPtr->chSelect, gPtr->wantVS1053);
								prgChange(gPtr->prgInst[gPtr->chSelect],gPtr->chSelect, !gPtr->wantVS1053);
							}
						}
					if(gPtr->chipChoice==1 && gPtr->sidInstChoice<(sid_instrumentsSize-1)) //SID
					{
					modalMoveDown(gPtr, shiftHit);
					sid_setInstrumentAllChannels(gPtr->sidInstChoice);
					}
					if(gPtr->chipChoice==3 && gPtr->opl3InstChoice<(opl3_instrumentsSize-1))  //OPL3
					{
						modalMoveDown(gPtr, shiftHit);
						opl3_setInstrumentAllChannels(gPtr->opl3InstChoice);
					}
				}
			break;
		case 0xb8: //left arrow
			if(instSelectMode==false)
					{
					if(note > (0 + shiftHit*11))
						{
						if(gPtr->chipChoice > 0) dispatchNote(false, 0, note+0x15, 0, false, gPtr->chipChoice, false, 0);
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
						if(gPtr->chipChoice > 0) dispatchNote(false, 0, note+0x15, 0, false, gPtr->chipChoice, false, 0);
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
				dispatchNote(true, 0x90 | gPtr->chSelect ,note+0x15,0x7F, gPtr->wantVS1053, gPtr->chipChoice, false, 0);
				//keep track of that note so we can Note_Off it when needed
				oldNote = note+0x15; //make it possible to do the proper NoteOff when the timer expires
				}
				else
					{
	
					dispatchNote(true, 0,note+0x15,0,false, gPtr->chipChoice, false, 0);
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
				for(j=0;j<theBeats[gPtr->selectBeat].howManyChans;j++) theBeats[gPtr->selectBeat].index[j]=0;
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
				for(j=0;j<theBeats[gPtr->selectBeat].howManyChans;j++) theBeats[gPtr->selectBeat].index[j]=0;
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
			if(gPtr->wantVS1053) chipAct[0]=0;
			else chipAct[1]=0;
	
			showMIDIChoiceText(gPtr);
			break;
		case 99: // C - chip select mode: 0=MIDI, 1=SID, (todo) 2= PSG, (todo) 3=OPL3
		if(instSelectMode==false){

			gPtr->chipChoice+=1;
			if(gPtr->chipChoice==1)
			{
				prepSIDinstruments(); //just arrived in sid, prep sid
				chipAct[0]=0; chipAct[1]=0;
			}
			if(gPtr->chipChoice==2)
			{
				clearSIDRegisters();
				emptySIDBuffer();
				chipAct[2]=0;
			}
			if(gPtr->chipChoice==3)
			{
				emptyPSGBuffer();
				shutPSG();
				chipAct[3]=0;
			}
			if(gPtr->chipChoice>3)
			{
				opl3_quietAll();
				emptyOPL3Buffer();
				gPtr->chipChoice=0; //loop back to midi at the start of the cyle
				chipAct[4]=0;
			}
			showChipChoiceText(gPtr);
					}
			break;
		default:
				//Send a Note
				if(gPtr->chipChoice !=3)
				{
				dispatchNote(true, 0x90 | gPtr->chSelect ,KeyCodesToMIDINotes[kernelEventData.key.raw],0x7F, gPtr->wantVS1053, gPtr->chipChoice, false, 0);
				//keep track of that note so we can Note_Off it when needed
				oldNote = note+0x15; //make it possible to do the proper NoteOff when the timer expires
				}
				else
					{
	
					dispatchNote(true, 0,KeyCodesToMIDINotes[kernelEventData.key.raw],0,false, gPtr->chipChoice, false, 0);
					}
		break;
	}
	//the following line can be used to get keyboard codes
	//printf("\n %03d 0x%02x    ",kernelEventData.key.raw,kernelEventData.key.raw);
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
		dispatchNote(false, gPtr->chSelect ,note+0x15,0x3F, gPtr->wantVS1053, gPtr->chipChoice, false, 0);
		break;
	}
}


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
	uint8_t noteScoop=0, delayScoop=0;
	struct aT *theT;
	
	theT = malloc(sizeof(aTrack));
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
					textGotoXY(5,4);textPrint("                        ");textGotoXY(5,4);
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
									dispatchNote(false, gPtr->chSelect,lastNote,recByte<VELO_MIN?VELO_MIN:recByte, gPtr->wantVS1053, gPtr->chipChoice, false, 0);
									}
								break;
							case 2:
								if(psgActive)
									{
									dispatchNote(false, gPtr->chSelect,lastNote,recByte<VELO_MIN?VELO_MIN:recByte, gPtr->wantVS1053, gPtr->chipChoice, false, 0);
									}
								break;
							case 3:
								if(opl3Active)
									{
									dispatchNote(false, gPtr->chSelect,lastNote,recByte<VELO_MIN?VELO_MIN:recByte, gPtr->wantVS1053, gPtr->chipChoice, false, 0);
									}
								break;
							}
						}
					dispatchNote(isHit, gPtr->chSelect,storedNote,recByte<VELO_MIN?VELO_MIN:recByte, gPtr->wantVS1053, gPtr->chipChoice, false, 0); //do the note or turn off the note
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
					if(isHit) {graphicsDefineColor(0, detectedNote,0xFF,0x00,0xFF); //paint it as a hit note
					textGotoXY(0,57);textPrintInt(recByte);
					}
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
					if(instSelectMode==false) refreshChipAct(chipAct);
					break;
				case TIMER_BEAT_1A ... 255:
					if(theBeats[gPtr->selectBeat].isActive)
						{
						switch(kernelEventData.timer.cookie)
							{
								default:
								j = kernelEventData.timer.cookie -TIMER_BEAT_1A; //find the right track of the beat to deal with
								break;
							}
	
						if (theBeats[gPtr->selectBeat].pending2x[j]== 2)theBeats[gPtr->selectBeat].pending2x[j]= 0; //restore for next pass
	
						if (theBeats[gPtr->selectBeat].pending2x[j]== 1){
							theBeats[gPtr->selectBeat].timers[j].absolute = getTimerAbsolute(TIMER_FRAMES) + mainTempoLUT[delayScoop];
							setTimer(&(theBeats[gPtr->selectBeat].timers[j]));
							theBeats[gPtr->selectBeat].pending2x[j]= 2;
							break;
							}

	
						getBeatTrackNoteInfo(theBeats, gPtr->selectBeat, j, &noteScoop, &delayScoop, theT);
						dispatchNote(false, theT->chan, noteScoop&0x7F,0x7F, gPtr->wantVS1053, theT->chip, true, theT->inst); // silence old note
						theBeats[gPtr->selectBeat].activeCount-=1;
	
						//textGotoXY(0,16);printf("activeCount %d",theBeats[gPtr->selectBeat].activeCount);
	
						//check if we need to expire the beat and act to die things down
						if(needsToWaitExpiration)
							{
	
								if(theBeats[gPtr->selectBeat].activeCount > 0)
								{
									break;
								}
								else
								{
									//textGotoXY(0,17);printf("reached end of activeCount");
									needsToWaitExpiration = false;
									theBeats[gPtr->selectBeat].isActive = false;
									if(theBeats[gPtr->selectBeat].pendingRelaunch) //oops, need to relaunch it at the end of it dying down
									{
										theBeats[gPtr->selectBeat].pendingRelaunch = false; //the relaunch will happen, so stop further relaunches for now
										launchBeat();
									}
									break;
								}
							}
							//otherwise proceed as normal and get the next note of that beat's track
						else {
	
							theBeats[gPtr->selectBeat].index[j]++;
							getBeatTrackNoteInfo(theBeats, gPtr->selectBeat, j, &noteScoop, &delayScoop, theT);
							if(theBeats[gPtr->selectBeat].index[j] ==  theT->count)
								{
								theBeats[gPtr->selectBeat].index[j] = 0;
								getBeatTrackNoteInfo(theBeats, gPtr->selectBeat, j, &noteScoop, &delayScoop, theT);
								}
	
	
							if((noteScoop&0x80)==0x80) {
								if (theBeats[gPtr->selectBeat].pending2x[j]==0) theBeats[gPtr->selectBeat].pending2x[j]=1;
								}
	
							dispatchNote(true,  theT->chan, noteScoop&0x7F, theT->chip==2?PSG_DEFAULT_VOL:0x7F, gPtr->wantVS1053,  theT->chip, true, theT->inst);
							theBeats[gPtr->selectBeat].activeCount++;
							theBeats[gPtr->selectBeat].timers[j].absolute = getTimerAbsolute(TIMER_FRAMES) + mainTempoLUT[delayScoop];
							setTimer(&(theBeats[gPtr->selectBeat].timers[j]));
							}
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
	
free(theT);
return 0;}
