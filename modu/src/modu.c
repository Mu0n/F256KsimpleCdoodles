#define F256LIB_IMPLEMENTATION

#define SPR_BASE 0x10000

//Chip selection high level
#define ACT_MID 0
#define ACT_SID 1
#define ACT_PSG 2
#define ACT_OPL 3


//SID area
//SID waveform
#define ACT_SID_TRI 24
#define ACT_SID_SAW 25
#define ACT_SID_PUL 26
#define ACT_SID_NOI 27

//SID wave shape ADSR
#define ACT_SID_A 28
#define ACT_SID_D 29
#define ACT_SID_S 30
#define ACT_SID_R 31	
#define ACT_SID_A_L  32
#define ACT_SID_D_L  33
#define ACT_SID_S_L  34
#define ACT_SID_R_L  35

//SID pulse width mod
#define ACT_SID_PWMHI 36
#define ACT_SID_PWMLO 37
#define ACT_SID_PWML1 38
#define ACT_SID_PWML2 39
#define ACT_SID_PWML3 40


//OPL3 area
//OPL3 operator config
#define ACT_OPL_OPL2      128 //radio button to stay in opl2
#define ACT_OPL_OPL3_03   129 //enable 4 operator in ops 0 to 3

//OPL3 waveform selection
#define ACT_OPL_WLEFT     130
#define ACT_OPL_WRIGHT    131

//OPL3 chip wide misc
#define ACT_OPL_TREMO     132
#define ACT_OPL_VIBRA     133
#define ACT_OPL_PERCM     134

//OPL3 channel wide
#define ACT_OPL_FMF   135 //feedback modulation factor, 3 bits
#define ACT_OPL_AMFM  136

//OPL3 wave shape ADSR	
#define ACT_OPL_MA 137
#define ACT_OPL_MS 138
#define ACT_OPL_MD 139
#define ACT_OPL_MR 140
#define ACT_OPL_MA_L 141
#define ACT_OPL_MS_L 142
#define ACT_OPL_MD_L 143
#define ACT_OPL_MR_L 144

#define ACT_OPL_CA 145
#define ACT_OPL_CS 146
#define ACT_OPL_CD 147
#define ACT_OPL_CR 148
#define ACT_OPL_CA_L 149
#define ACT_OPL_CS_L 150
#define ACT_OPL_CD_L 151
#define ACT_OPL_CR_L 152

#define ACT_OPL_TA 153
#define ACT_OPL_TS 154
#define ACT_OPL_TD 155
#define ACT_OPL_TR 156
#define ACT_OPL_TA_L 157
#define ACT_OPL_TS_L 158
#define ACT_OPL_TD_L 159
#define ACT_OPL_TR_L 160

#define ACT_OPL_FA 161
#define ACT_OPL_FS 162
#define ACT_OPL_FD 163
#define ACT_OPL_FR 164
#define ACT_OPL_FA_L 165
#define ACT_OPL_FS_L 166
#define ACT_OPL_FD_L 167
#define ACT_OPL_FR_L 168





#include "f256lib.h"
#include "../src/mumouse.h"
#include "../src/musid.h"
#include "../src/muMidi.h"
#include "../src/mudispatch.h"
#include "../src/moduUI.h"
#include "../src/muopl3.h"
#include "../src/mupsg.h"

#define GUI_N_RADIOS 8
#define GUI_N_SLIDERS 4
#define GUI_N_DIALS 4

typedef struct sliderActivity{
	uint8_t index;
	int16_t iMX, iMY;
	uint8_t value8;
	uint16_t value16;
} sliderActivity;
typedef struct dialActivity{
	uint8_t index;
	int16_t iMX, iMY;
	uint8_t value8;
	uint16_t value16;
} dialActivity;
	
uint8_t sidSpriteAction[64] = {ACT_MID, ACT_SID, ACT_PSG, ACT_OPL, ACT_SID_TRI, ACT_SID_SAW, ACT_SID_PUL, ACT_SID_NOI,
							   ACT_SID_A, 0xFF, ACT_SID_D, 0xFF, ACT_SID_S, 0xFF, ACT_SID_R, 0xFF,
							   ACT_SID_A_L, ACT_SID_D_L, ACT_SID_S_L, ACT_SID_R_L, ACT_SID_PWMHI, ACT_SID_PWMLO, ACT_SID_PWML1, ACT_SID_PWML2,
							   ACT_SID_PWML3, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
							   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
							   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
							   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
							   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

uint8_t oplSpriteAction[64] = {ACT_MID, ACT_SID, ACT_PSG, ACT_OPL, ACT_OPL_OPL2, ACT_OPL_OPL3_03, ACT_OPL_WLEFT, ACT_OPL_WRIGHT,
							   ACT_OPL_TREMO, ACT_OPL_VIBRA, ACT_OPL_PERCM, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
							   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
							   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
							   ACT_OPL_MA, ACT_OPL_MS, ACT_OPL_MD, ACT_OPL_MR, ACT_OPL_MA_L, ACT_OPL_MS_L, ACT_OPL_MD_L, ACT_OPL_MR_L,
							   ACT_OPL_CA, ACT_OPL_CS, ACT_OPL_CD, ACT_OPL_CR, ACT_OPL_CA_L, ACT_OPL_CS_L, ACT_OPL_CD_L, ACT_OPL_CR_L,
							   ACT_OPL_TA, ACT_OPL_TS, ACT_OPL_TD, ACT_OPL_TR, ACT_OPL_TA_L, ACT_OPL_TS_L, ACT_OPL_TD_L, ACT_OPL_TR_L,
							   ACT_OPL_FA, ACT_OPL_FS, ACT_OPL_FD, ACT_OPL_FR, ACT_OPL_FA_L, ACT_OPL_FS_L, ACT_OPL_FD_L, ACT_OPL_FR_L
};

void dispatchAction(struct generic_UI *, bool);
void resetActivity(void);

EMBED(gui, "../assets/gui.bin", 0x10000); //4kb
EMBED(pal, "../assets/gui.pal", 0x11000); //1kb
EMBED(pianopal, "../assets/piano.pal", 0x30000);
EMBED(keys, "../assets/piano.raw", 0x30400);

int16_t mX, mY; //mouse coordinates

struct sliderActivity sliAct;
struct dialActivity   diaAct;

struct radioB_UI radios[8];
struct slider_UI sliders[4];
struct generic_UI sliders_labels[7];
struct dial_UI dials[2];
bool noteColors[88]={1,0,1, 1,0,1,0,1, 1,0,1,0,1,0,1, 1,0,1,0,1, 1,0,1,0,1,0,1, 1,0,1,0,1, 1,0,1,0,1,0,1, 1,0,1,0,1, 1,0,1,0,1,0,1, 1,0,1,0,1, 1,0,1,0,1,0,1, 1,0,1,0,1, 1,0,1,0,1,0,1, 1,0,1,0,1, 1,0,1,0,1,0,1, 1};


void textLayerSID()
{
}
void textLayerOPL()
{
}
void textLayerGen()
{
	textGotoXY(10,4);textPrint("MIDI");
	textGotoXY(10,5);textPrint("SID");
	textGotoXY(10,6);textPrint("PSG");
	textGotoXY(10,7);textPrint("OPL3");
	
	textGotoXY(30,4);textPrint("TRI");
	textGotoXY(30,5);textPrint("SAW");
	textGotoXY(30,6);textPrint("PUL");
	textGotoXY(30,7);textPrint("NOI");
	
	textGotoXY(17,3);textPrint("A D S R");
	
	textGotoXY(22,9);textPrint("PWM");
}

void loadGUIOPL()
{
}

void loadGUISID()
{
	uint8_t sprSoFar = 0;
	
	textLayerGen();
	textLayerSID();
	
	//radio button group 0: switch between chips radio buttons
	for(uint8_t i=0; i<4; i++)
		{
		setGeneric(sprSoFar, 64, 64+8*i, SPR_BASE+UI_SWTCH, i, 8, 0,6,2,5, sidSpriteAction[sprSoFar], &(radios[i].gen));
		setRadioB(&radios[i], true, 0, i==1?true:false); // groupID 0
		sprSoFar++;
		}

	//SID
			
	//radio button group 1: SID waveform
	for(uint8_t i=4; i<8; i++)
	{
	setGeneric(sprSoFar, 144, 64+8*(i-4), SPR_BASE+UI_SWTCH, i, 8, 0,6,2,5, sidSpriteAction[sprSoFar], &(radios[i].gen));
	setRadioB(&radios[i], true, 1, i==5?true:false);
	sprSoFar++;
	}
	
	//sliders for ASDR for SID
	for(uint8_t i=0; i<4; i++)
	{
	setGeneric(sprSoFar , 99+i*8 , 64, SPR_BASE+UI_SLIDS, i, 8, 1,5,3,12, sidSpriteAction[sprSoFar], &(sliders[i].gen));
	sprSoFar+=2;
	}
	setSlider(&sliders[0], (gPtr->sidValues->ad & 0xF0)>>4, 0, 15, 0, 0, 0, SPR_BASE); //A
	setSlider(&sliders[1], (gPtr->sidValues->ad & 0x0F),    0, 15, 0, 0, 0, SPR_BASE); //D
	setSlider(&sliders[2], (gPtr->sidValues->sr & 0xF0)>>4, 0, 15, 0, 0, 0, SPR_BASE); //S
	setSlider(&sliders[3], (gPtr->sidValues->sr & 0x0F),    0, 15, 0, 0, 0, SPR_BASE); //R
	
	for(uint8_t i=0; i<4; i++) //and their labels
	{	
	setGeneric(sprSoFar, 99+i*8 , 79, SPR_BASE, i, 8, 0,0,0,0, sidSpriteAction[sprSoFar], &(sliders_labels[i]));
	showGeneric(&(sliders_labels[i]));
	updateGeneric(&(sliders_labels[i]), sliders[i].value8, SPR_BASE);
	sprSoFar++;
	}
	
	//dial for pulse width
	setGeneric(sprSoFar, 98, 99, SPR_BASE+UI_DIALS, 0, 8, 0,6,0,6, sidSpriteAction[sprSoFar],&(dials[0].gen));
	setDial(&dials[0], (gPtr->sidValues->pwdHi & 0x0F), 0, 15, 0, 0, 0, SPR_BASE);
	sprSoFar++;
	setGeneric(sprSoFar, 109, 99, SPR_BASE+UI_DIALS, 1, 8, 0,6,0,6, sidSpriteAction[sprSoFar],&(dials[1].gen));
	setDial(&dials[1], gPtr->sidValues->pwdLo , 0, 255, 0, 0, 0, SPR_BASE);
	sprSoFar++;
	
	setGeneric(sprSoFar, 97 , 108, SPR_BASE, 4, 8, 0,0,0,0, sidSpriteAction[sprSoFar], &(sliders_labels[4]));
	showGeneric(&(sliders_labels[4]));
	updateGeneric(&(sliders_labels[4]), dials[0].value8, SPR_BASE);
	sprSoFar++;
	setGeneric(sprSoFar, 103 , 108, SPR_BASE, 5, 8, 0,0,0,0, sidSpriteAction[sprSoFar], &(sliders_labels[5]));
	showGeneric(&(sliders_labels[5]));
	updateGeneric(&(sliders_labels[5]), (dials[1].value8 & 0xF0)>>4, SPR_BASE);
	sprSoFar++;
	setGeneric(sprSoFar, 109 , 108, SPR_BASE, 6, 8, 0,0,0,0, sidSpriteAction[sprSoFar], &(sliders_labels[6]));
	showGeneric(&(sliders_labels[6]));
	updateGeneric(&(sliders_labels[6]), dials[1].value8 & 0x0F, SPR_BASE);
	sprSoFar++;
	
	
}

void backgroundSetup()
{
	uint16_t c;
	
	POKE(MMU_IO_CTRL, 0x00);
	// XXX GAMMA  SPRITE   TILE  | BITMAP  GRAPH  OVRLY  TEXT
	POKE(VKY_MSTR_CTRL_0, 0b00101111); //sprite,graph,overlay,text
	// XXX XXX  FON_SET FON_OVLY | MON_SLP DBL_Y  DBL_X  CLK_70
	POKE(VKY_MSTR_CTRL_1, 0b00000100); //font overlay, double height text, 320x240 at 60 Hz;
	POKE(VKY_LAYER_CTRL_0, 0b00000001); //bitmap 1 in layer 0, bitmap 0 in layer 1
	POKE(VKY_LAYER_CTRL_1, 0b00000010); //bitmap 2 in layer 2

	POKE(0xD00D,0x00); //force black graphics background
	POKE(0xD00E,0x00);
	POKE(0xD00F,0x00);
//palette
	POKE(MMU_IO_CTRL,1);  //MMU I/O to page 1
	//prep to copy over the palette to the CLUT
	for(c=0;c<1023;c++) 
	{
		POKE(VKY_GR_CLUT_0+c, FAR_PEEK(0x11000+c)); //palette for GUI
	}
	for(c=0;c<1023;c++) 
	{
		POKE(VKY_GR_CLUT_1+c, FAR_PEEK(0x30000+c)); //palette for piano
	}
	
	POKE(MMU_IO_CTRL,0); //MMU I/O to page 0

	
	bitmapSetActive(0);
	bitmapSetCLUT(0);
	bitmapSetColor(3);
	bitmapClear();
	
	bitmapSetActive(1);
	bitmapSetCLUT(1);
	bitmapClear();
	bitmapSetAddress(1,0x30400);

	bitmapSetVisible(0,true);
	bitmapSetVisible(1,true);
	bitmapSetVisible(2,false);
}

void resetActivity()
{
	diaAct.index = 0xFF;
	diaAct.value8 = 0;
	diaAct.value16 = 0;
	sliAct.index = 0xFF;
	sliAct.value8 = 0;
	sliAct.value16 = 0;
}
void setup()
	{
	backgroundSetup();

	//set a structure of globals related to sound dispatching
	gPtr = malloc(sizeof(globalThings));
	resetGlobals(gPtr);	

	//SID prep
	clearSIDRegisters();
	sid_setInstrumentAllChannels(0);
	setMonoSID();
		
	//Prep PSG stuff
	setMonoPSG();

	//Prep OPL3 stuff
	opl3_initialize();
	opl3_setInstrumentAllChannels(0);

	prepMouse();
	
	//global values
	gPtr = malloc(sizeof(globalThings));
	resetGlobals(gPtr);
	gPtr->chipChoice = 1;

	gPtr->sidValues->maxVolume = 0x0F;
	gPtr->sidValues->pwdLo = 0x88;
	gPtr->sidValues->pwdHi = 0x02;
	gPtr->sidValues->ad = 0x19;
	gPtr->sidValues->sr = 0x23;
	gPtr->sidValues->ctrl = 0x20;
	gPtr->sidValues->fcfLo = 0x00;
	gPtr->sidValues->fcfHi = 0x00;
	gPtr->sidValues->frr = 0x00;
	
	
	loadGUISID();
	resetActivity();
	}

void dispatchAction(struct generic_UI *gen, bool isClicked) //here we dispatch what the click behavior means in this specific project
{
	uint8_t c;
	bool wChange = false;
	switch(gen->actionID)
	{
		case ACT_MID ... ACT_OPL:
		case ACT_SID_TRI ... ACT_SID_NOI:
			gen->isClicked = !gen->isClicked;

			updateRadioB(&radios[gen->parentIndex]); //toggle the current one
			if(gen->isClicked && radios[gen->parentIndex].isGroupExclusive) //start toggling others off if it was excl
			{
				for(uint8_t j=0; j<GUI_N_RADIOS; j++)
				{
					if(gen->parentIndex==j) continue; //leave the current one alone
					if(radios[j].isGroupExclusive && radios[gen->parentIndex].groupID == radios[j].groupID) //if they are part of the same group
						{
							if(radios[j].gen.isClicked)
							{
							radios[j].gen.isClicked = false;
							updateRadioB(&radios[j]);
							}
						}
				}
			}
			if(gen->actionID >= ACT_MID && gen->actionID <= ACT_OPL) gPtr->chipChoice = gen->actionID; //change the chip
			if(gen->actionID == ACT_SID_TRI) {gPtr->sidValues->ctrl = ((gPtr->sidValues->ctrl) & 0x0F) | 0x10;wChange=true;}
			if(gen->actionID == ACT_SID_SAW) {gPtr->sidValues->ctrl = ((gPtr->sidValues->ctrl) & 0x0F) | 0x20;wChange=true;}
			if(gen->actionID == ACT_SID_PUL) {gPtr->sidValues->ctrl = ((gPtr->sidValues->ctrl) & 0x0F) | 0x40;wChange=true;}
			if(gen->actionID == ACT_SID_NOI) {gPtr->sidValues->ctrl = ((gPtr->sidValues->ctrl) & 0x0F) | 0x80;wChange=true;}
			
		//textGotoXY(33,3);printf("%02x ",gPtr->sidValues->ctrl);
			if(wChange){
				for(c=0;c<3;c++)
					{
					sid_setInstrument(0,c,*(gPtr->sidValues));
					sid_setInstrument(1,c,*(gPtr->sidValues));
					}
				}
			break;
		
		case ACT_SID_A ... ACT_SID_R:
			gen->isClicked = true;
			sliAct.iMX = mX;
			sliAct.iMY = mY;
			sliAct.index = gen->parentIndex;
			hideMouse();
			break;
		case ACT_SID_PWMHI ... ACT_SID_PWMLO:
			gen->isClicked = true;
			diaAct.iMX = mX;
			diaAct.iMY = mY;
			diaAct.index = gen->parentIndex;
			hideMouse();
			break;
	}
}

void checkUIEClick(uint16_t newX, uint16_t newY, struct generic_UI *gen) //uses the generic part to verify if it was clicked
{

	if(newX >= (gen->x + gen->x1) && newX <= (gen->x + gen->x2))
	{
		if(newY >= (gen->y + gen->y1) && newY <= (gen->y + gen->y2)) //multi stage check to speed it up
		{
			dispatchAction(gen, gen->isClicked);
		}
	}

}


void checkUIClicks(uint16_t newX,uint16_t newY) //parse every clickable element
{
	for(uint8_t i=0; i<GUI_N_RADIOS; i++) 
	{
		checkUIEClick(newX, newY, &(radios[i].gen)); //check this group of radio buttons
	}
	for(uint8_t i=0; i<GUI_N_SLIDERS; i++) 
	{
		checkUIEClick(newX, newY, &(sliders[i].gen)); //check this group of sliders
	}	
	for(uint8_t i=0; i<GUI_N_DIALS; i++) 
	{
		checkUIEClick(newX, newY, &(dials[i].gen)); //check this group of sliders
	}
}
		
void updateChip(struct slider_UI *sli)
{
	switch(sli->gen.actionID)
	{
		case ACT_SID_A:
			gPtr->sidValues->ad = (gPtr->sidValues->ad&0x0F) | (sli->value8<<4);
			break;
		case ACT_SID_D:
			gPtr->sidValues->ad = (gPtr->sidValues->ad&0xF0) | (sli->value8);
			break;
		case ACT_SID_S:
			gPtr->sidValues->sr = (gPtr->sidValues->sr&0x0F) | (sli->value8<<4);
			break;
		case ACT_SID_R:
			gPtr->sidValues->sr = (gPtr->sidValues->sr&0xF0) | (sli->value8);
			break;
	}
	for(uint8_t c=0;c<3;c++)
	{
	sid_setInstrument(0,c,*(gPtr->sidValues));
	sid_setInstrument(1,c,*(gPtr->sidValues));
	}
}
void updateChipDial(struct dial_UI *dia)
{
	switch(dia->gen.actionID)
	{
		case ACT_SID_PWMLO:
			gPtr->sidValues->pwdLo = dia->value8;
			break;
		case ACT_SID_PWMHI:
			gPtr->sidValues->pwdHi = dia->value8 & 0x0F;
			break;
	}
	for(uint8_t c=0;c<3;c++)
	{
	sid_setInstrument(0,c,*(gPtr->sidValues));
	sid_setInstrument(1,c,*(gPtr->sidValues));
	}
}
void checkMIDIIn()
{
}
	
int main(int argc, char *argv[]) {
uint16_t midiPending;
uint8_t recByte, detectedNote, detectedColor, lastCmd=0x90; //for MIDI In event detection
bool mPressed = false; //current latch status of left mouse click. if you long press, it won't trigger many multiples of the event
bool nextIsNote = false; //detect a 0x9? or 0x8? command, the next is a note byte, used for coloring the keyboard note-rects
bool nextIsSpeed = false; //detects if we're at the end of a note on or off trio of bytes
bool isHit = false; // true is hit, false is released, distinguishes betwee Note On and Note Off when they happen
	
bool opl3Active = false;
bool psgActive = false;
bool sidActive = false;
uint8_t storedNote; //stored values when twinlinked
uint8_t lastNote = 0; // for monophonic chips, store this to mute last note before doing 

uint16_t i;

setup();

	while(true)
	{
	//MIDI in
	if(!(PEEK(MIDI_CTRL) & 0x02)) //rx not empty
		{
			midiPending = PEEKW(MIDI_RXD) & 0x0FFF; //discard top 4 bits of MIDI_RXD+1
			//deal with the MIDI bytes and exhaust the FIFO buffer
				for(i=0; i<midiPending; i++)
					{
					//get the next MIDI in FIFO buffer byte
					recByte=PEEK(MIDI_FIFO);
					if(recByte == 0xfe) continue; //active sense, ignored
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
										dispatchNote(false,0,lastNote,recByte, gPtr->wantVS1053, gPtr->chipChoice, false, 0);
										}
									break;
								case 2:
									if(psgActive) 
										{
										dispatchNote(false,0,lastNote,recByte, gPtr->wantVS1053, gPtr->chipChoice, false, 0);
										}
									break;
								case 3:
									if(opl3Active) 
										{
										dispatchNote(false,0,lastNote,recByte, gPtr->wantVS1053, gPtr->chipChoice, false, 0);
										}
									break;
								}
							}
						dispatchNote(isHit,0,storedNote,0x7F, gPtr->wantVS1053, gPtr->chipChoice, false, 0); //do the note or turn off the note
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
						if(isHit) {graphicsDefineColor(1, detectedNote,0xFF,0x00,0xFF); //paint it as a hit note
						//textGotoXY(0,20);textPrintInt(recByte);
						}
						//otherwise it's a 0x80 'NoteOff' command
						else {
							detectedColor = noteColors[detectedNote-1]?0xFF:0x00;
							graphicsDefineColor(1, detectedNote,detectedColor,detectedColor,detectedColor); //swap back the original color according to this look up ref table noteColors
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
					}
				
		}	
		
	//events	
	kernelNextEvent();
	if(kernelEventData.type == kernelEvent(mouse.CLICKS))
	{
		//dispatchNote(bool isOn, uint8_t channel, uint8_t note, uint8_t speed, bool wantAlt, uint8_t whichChip, bool isBeat, uint8_t beatChan)
	}
	else if(kernelEventData.type == kernelEvent(mouse.DELTA))
	{
		mX = PEEKW(PS2_M_X_LO)+(int8_t)kernelEventData.mouse.delta.x;
		mY = PEEKW(PS2_M_Y_LO)+(int8_t)kernelEventData.mouse.delta.y;
		
		if(mX<0) mX=0; if(mX>640-16) mX=640-16;
		if(mY<0) mY=0; if(mY>480-16) mY=480-16;
		POKEW(PS2_M_X_LO,mX);
        POKEW(PS2_M_Y_LO,mY);

		if(sliAct.index != 0xFF)  //keep track of slider technology
			{
				int16_t temp;
				temp = (int16_t)sliders[sliAct.index].value8;
				temp -= (int8_t)kernelEventData.mouse.delta.y;
				if(temp < (int16_t)sliders[sliAct.index].min8) temp = sliders[sliAct.index].min8;
				if(temp > (int16_t)sliders[sliAct.index].max8) temp = sliders[sliAct.index].max8;
				
				sliders[sliAct.index].value8 = (uint8_t)temp;
				updateSlider(&(sliders[sliAct.index]),SPR_BASE);
				updateChip(&(sliders[sliAct.index]));
				updateGeneric(&(sliders_labels[sliAct.index]),sliders[sliAct.index].value8,SPR_BASE);
			}
		if(diaAct.index != 0xFF)  //keep track of dial technology
			{
				int16_t temp;
				temp = (int16_t)dials[diaAct.index].value8;
				temp += (int8_t)kernelEventData.mouse.delta.x;
				if(temp < (int16_t)dials[diaAct.index].min8) temp = dials[diaAct.index].min8;
				if(temp > (int16_t)dials[diaAct.index].max8) temp = dials[diaAct.index].max8;
				
				dials[diaAct.index].value8 = (uint8_t)temp;
				updateDial(&(dials[diaAct.index]),SPR_BASE);
				updateChipDial(&(dials[diaAct.index]));
				if(dials[diaAct.index].gen.actionID == ACT_SID_PWMHI) updateGeneric(&(sliders_labels[4]),dials[0].value8,SPR_BASE);
				if(dials[diaAct.index].gen.actionID == ACT_SID_PWMLO) 
					{ 
					updateGeneric(&(sliders_labels[5]),(dials[1].value8&0xF0)>>4,SPR_BASE);
					updateGeneric(&(sliders_labels[6]),dials[1].value8&0x0F,SPR_BASE);	
					}
			}
		if((kernelEventData.mouse.delta.buttons&0x01)==0x01 && mPressed==false)
			{
				mPressed=true; //activate the latch
				checkUIClicks((mX>>1)+32,(mY>>1)+32);
			}
		if(mPressed==true && (kernelEventData.mouse.delta.buttons&0x01)==0x00)
			{
				if(sliAct.index != 0xFF) 
					{
						sliders[sliAct.index].gen.isClicked = false;
						sliAct.index = 0xFF;
						showMouse();
					}
				if(diaAct.index != 0xFF) 
					{
						dials[diaAct.index].gen.isClicked = false;
						diaAct.index = 0xFF;
						showMouse();
					}
				mPressed=false; //release the latch
			}
		}
	}
	
return 0;}
