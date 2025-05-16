#define F256LIB_IMPLEMENTATION

#define MAJORVER 0
#define MINORVER 1

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

#define TIMER_FRAMES 0
#define TIMER_POLY_DELAY 1
#define TIMER_POLY_COOKIE 1

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
#define SIZE8 8
#define SIZE16 16
#define SPR_BASE  0x10000
#define SPR_BASE8 0x14000
#define SPR_BASE16 0x10000
EMBED(gui16, "../assets/gui16.bin", 0x10000); //16kb
EMBED(gui8,  "../assets/gui8.bin" , 0x14000); //4kb
EMBED(pal,   "../assets/gui.pal",     0x15000); //1kb
EMBED(pianopal, "../assets/piano.pal", 0x30000);
EMBED(keys, "../assets/piano.raw", 0x30400);

int16_t mX, mY; //mouse coordinates

struct sliderActivity sliAct;
struct dialActivity   diaAct;

struct timer_t polyTimer;

struct radioB_UI radios[8];
struct slider_UI sliders[4];
struct generic_UI sliders_labels[7];
struct lighter_UI lights[21];
struct dial_UI dials[2];
bool noteColors[88]={1,0,1, 1,0,1,0,1, 1,0,1,0,1,0,1, 1,0,1,0,1, 1,0,1,0,1,0,1, 1,0,1,0,1, 1,0,1,0,1,0,1, 1,0,1,0,1, 1,0,1,0,1,0,1, 1,0,1,0,1, 1,0,1,0,1,0,1, 1,0,1,0,1, 1,0,1,0,1,0,1, 1,0,1,0,1, 1,0,1,0,1,0,1, 1};


//Sends a kernel based timer. You must prepare a timer_t struct first and initialize its fields
bool setTimer(const struct timer_t *timer)
{
    *(uint8_t*)0xf3 = timer->units;
    *(uint8_t*)0xf4 = timer->absolute;
    *(uint8_t*)0xf5 = timer->cookie;
    kernelCall(Clock.SetTimer);
	return !kernelError;
}
//getTimerAbsolute:
//This is essential if you want to retrigger a timer properly. The old value of the absolute
//field has a high chance of being desynchronized when you arrive at the moment when a timer
//is expired and you must act upon it.
//get the value returned by this, add the delay you want, and use setTimer to send it off
//ex: myTimer.absolute = getTimerAbsolute(TIMES_SECONDS) + TIMER_MYTIMER_DELAY
uint8_t getTimerAbsolute(uint8_t units)
{
    *(uint8_t*)0xf3 = units | 0x80;
    return kernelCall(Clock.SetTimer);
}

void textLayerMIDI()
{	
	textGotoXY(12,28);textPrint("Infinite!");
}
void textLayerPSG()
{
}
void textLayerSID()
{
	textGotoXY(30,2);textPrint("TRI");
	textGotoXY(30,4);textPrint("SAW");
	textGotoXY(30,6);textPrint("PUL");
	textGotoXY(30,8);textPrint("NOI");
	
	textGotoXY(13,2);textPrint("A  D  S  R");
	
	textGotoXY(13,9);textPrint("PWM");
	
}
void textLayerOPL()
{
}
void textLayerGen()
{
	textGotoXY(0,0);printf("ChipForge v%d.%d by Mu0n",MAJORVER,MINORVER);
	textGotoXY(6,2);textPrint("MIDI");
	textGotoXY(6,4);textPrint("SID");
	textGotoXY(6,6);textPrint("PSG");
	textGotoXY(6,8);textPrint("OPL3");
	textGotoXY(1,28);textPrint("Polyphony:");
}

void clearText()
{
POKE(MMU_IO_CTRL,2);
for(uint16_t j=1;j<30;j++)
	{
	for(uint16_t i=0;i<80;i++)
		{
			POKE(0xC000 + (uint16_t)(j*80 + i), 32);
		}
	}
POKE(MMU_IO_CTRL,0);
}

void hideGUI()
{
	for(uint8_t i=4; i<64; i++) spriteSetVisible(i,false);
}
void loadGUIMIDI()
{
	uint8_t sprSoFar = 0;
	textLayerGen();
	textLayerMIDI();
}
void loadGUIPSG()
{	
	#define GROUP4_PSG_POLY_X 79
	#define GROUP4_PSG_POLY_Y 253
	#define GROUP4_PSG_POLY_SPACE 16
	uint8_t sprSoFar = 4;
	textLayerGen();
	textLayerPSG();
	
		//polyphony for OPL (9 indicators)
	for(uint8_t i=15;i<21; i++)
	{		
		setGeneric(sprSoFar,GROUP4_PSG_POLY_X + (i-15)*GROUP4_PSG_POLY_SPACE, GROUP4_PSG_POLY_Y, SPR_BASE+(uint32_t)(UI_STAT),i,SIZE16, 0,0,0,0, sidSpriteAction[sprSoFar], &(lights[i].gen));
		setLighter(&(lights[i]),0, SPR_BASE);
		sprSoFar++;
	}
	
}
void loadGUIOPL()
{
	#define GROUP4_OPL_POLY_X 79
	#define GROUP4_OPL_POLY_Y 253
	#define GROUP4_OPL_POLY_SPACE 16
	uint8_t sprSoFar = 4;
	textLayerGen();
	textLayerOPL();
	
	//polyphony for OPL (9 indicators)
	for(uint8_t i=6;i<15; i++)
	{		
		setGeneric(sprSoFar,GROUP4_OPL_POLY_X + (i-6)*GROUP4_OPL_POLY_SPACE, GROUP4_OPL_POLY_Y, SPR_BASE+(uint32_t)(UI_STAT),i,SIZE16, 0,0,0,0, sidSpriteAction[sprSoFar], &(lights[i].gen));
		setLighter(&(lights[i]),0, SPR_BASE);
		sprSoFar++;
	}
	
}

void loadGUISID()
{
	#define GROUP0_CHIP_RADS_X 40
	#define GROUP0_CHIP_RADS_Y 44
	#define GROUP0_CHIP_SPACE  16
	
	#define GROUP1_SID_ADSR_SLIDS_X 79
	#define GROUP1_SID_ADSR_SLIDS_Y 50
	#define GROUP1_SID_ADSR_SLIDS_SPACE 12
	#define GROUP1_SID_ADSR_SLIDSL_X GROUP1_SID_ADSR_SLIDS_X-1
	#define GROUP1_SID_ADSR_SLIDSL_Y GROUP1_SID_ADSR_SLIDS_Y+28
	#define GROUP1_SID_ADSR_SLIDSL_SPACE 12
	
	#define GROUP2_SID_WAVE_X 136
	#define GROUP2_SID_WAVE_Y GROUP0_CHIP_RADS_Y
	#define GROUP2_SID_WAVE_SPACE 16
	
	#define GROUP3_SID_PWM_X  98
	#define GROUP3_SID_PWM_Y  99
	#define GROUP3_SID_PWN_SPACE  16
	
	#define GROUP3_SID_PWML_X  GROUP3_SID_PWM_X-3
	#define GROUP3_SID_PWML_Y  GROUP3_SID_PWM_Y+18
	#define GROUP3_SID_PWML_SPACE  10
	
	
	#define GROUP4_SID_POLY_X 79
	#define GROUP4_SID_POLY_Y 253
	#define GROUP4_SID_POLY_SPACE 16
	#define GrOU
	uint8_t sprSoFar = 0;
	
	textLayerGen();
	textLayerSID();
	
	//radio button group 0: switch between chips radio buttons
	for(uint8_t i=0; i<4; i++)
		{
		setGeneric(sprSoFar, GROUP0_CHIP_RADS_X, GROUP0_CHIP_RADS_Y+GROUP0_CHIP_SPACE*i, SPR_BASE+(uint32_t)(UI_SWTCH*SIZE16*SIZE16), i, SIZE16, 0,13,4,11, sidSpriteAction[sprSoFar], &(radios[i].gen));
		setRadioB(&radios[i], true, 0, i==1?true:false); // groupID 0
		sprSoFar++;
		}

	//SID
			
	//radio button group 1: SID waveform
	for(uint8_t i=4; i<8; i++)
	{
	setGeneric(sprSoFar, GROUP2_SID_WAVE_X, GROUP2_SID_WAVE_Y+(i-4)*GROUP2_SID_WAVE_SPACE, SPR_BASE+(uint32_t)(UI_SWTCH*SIZE16*SIZE16), i, SIZE16, 0,13,4,11, sidSpriteAction[sprSoFar], &(radios[i].gen));
	setRadioB(&radios[i], true, 1, i==5?true:false);
	sprSoFar++;
	}
	
	//sliders for ASDR for SID
	for(uint8_t i=0; i<4; i++)
	{
	setGeneric(sprSoFar , GROUP1_SID_ADSR_SLIDS_X+i*GROUP1_SID_ADSR_SLIDS_SPACE , GROUP1_SID_ADSR_SLIDS_Y, SPR_BASE+(uint32_t)(UI_SLIDS*SIZE16*SIZE16), i, SIZE16, 3,10,6,25, sidSpriteAction[sprSoFar], &(sliders[i].gen));
	sprSoFar+=2;
	}
	setSlider(&sliders[0], (gPtr->sidValues->ad & 0xF0)>>4, 0, 15, 0, 0, 0, SPR_BASE); //A
	setSlider(&sliders[1], (gPtr->sidValues->ad & 0x0F),    0, 15, 0, 0, 0, SPR_BASE); //D
	setSlider(&sliders[2], (gPtr->sidValues->sr & 0xF0)>>4, 0, 15, 0, 0, 0, SPR_BASE); //S
	setSlider(&sliders[3], (gPtr->sidValues->sr & 0x0F),    0, 15, 0, 0, 0, SPR_BASE); //R
	
	for(uint8_t i=0; i<4; i++) //and their labels
	{	
	setGeneric(sprSoFar, GROUP1_SID_ADSR_SLIDSL_X+i*GROUP1_SID_ADSR_SLIDSL_SPACE , GROUP1_SID_ADSR_SLIDSL_Y, SPR_BASE, i, SIZE16, 0,0,0,0, sidSpriteAction[sprSoFar], &(sliders_labels[i]));
	showGeneric(&(sliders_labels[i]));
	updateGeneric(&(sliders_labels[i]), sliders[i].value8, SPR_BASE);
	sprSoFar++;
	}
	
	//dial for pulse width
	setGeneric(sprSoFar, GROUP3_SID_PWM_X, GROUP3_SID_PWM_Y, SPR_BASE+(uint32_t)(UI_DIALS*SIZE16*SIZE16), 0, SIZE16, 0,13,0,13, sidSpriteAction[sprSoFar],&(dials[0].gen));
	setDial(&dials[0], (gPtr->sidValues->pwdHi & 0x0F), 0, 15, 0, 0, 0, SPR_BASE);
	sprSoFar++;
	setGeneric(sprSoFar, GROUP3_SID_PWM_X+GROUP3_SID_PWN_SPACE, GROUP3_SID_PWM_Y, SPR_BASE+(uint32_t)(UI_DIALS*SIZE16*SIZE16), 1, SIZE16, 0,13,0,13, sidSpriteAction[sprSoFar],&(dials[1].gen));
	setDial(&dials[1], gPtr->sidValues->pwdLo , 0, 255, 0, 0, 0, SPR_BASE);
	sprSoFar++;
	
	setGeneric(sprSoFar, GROUP3_SID_PWML_X , GROUP3_SID_PWML_Y, SPR_BASE, 4, SIZE16, 0,0,0,0, sidSpriteAction[sprSoFar], &(sliders_labels[4]));
	showGeneric(&(sliders_labels[4]));
	updateGeneric(&(sliders_labels[4]), dials[0].value8, SPR_BASE);
	sprSoFar++;
	setGeneric(sprSoFar, GROUP3_SID_PWML_X+GROUP3_SID_PWML_SPACE , GROUP3_SID_PWML_Y, SPR_BASE, 5, SIZE16, 0,0,0,0, sidSpriteAction[sprSoFar], &(sliders_labels[5]));
	showGeneric(&(sliders_labels[5]));
	updateGeneric(&(sliders_labels[5]), (dials[1].value8 & 0xF0)>>4, SPR_BASE);
	sprSoFar++;
	setGeneric(sprSoFar, GROUP3_SID_PWML_X+2*GROUP3_SID_PWML_SPACE , GROUP3_SID_PWML_Y, SPR_BASE, 6, SIZE16, 0,0,0,0, sidSpriteAction[sprSoFar], &(sliders_labels[6]));
	showGeneric(&(sliders_labels[6]));
	updateGeneric(&(sliders_labels[6]), dials[1].value8 & 0x0F, SPR_BASE);
	sprSoFar++;
	
	//polyphony for SID (6 indicators)
	for(uint8_t i=0;i<6; i++)
	{		
		setGeneric(sprSoFar,GROUP4_SID_POLY_X + i*GROUP4_SID_POLY_SPACE, GROUP4_SID_POLY_Y, SPR_BASE+(uint32_t)(UI_STAT),i,SIZE16, 0,0,0,0, sidSpriteAction[sprSoFar], &(lights[i].gen));
		setLighter(&(lights[i]),0, SPR_BASE);
		sprSoFar++;
		
	}
	
	
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
		POKE(VKY_GR_CLUT_0+c, FAR_PEEK(0x15000+c)); //palette for GUI
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
	
	
	//set a structure of globals related to sound dispatching
	gPtr = malloc(sizeof(globalThings));
	resetGlobals(gPtr);
	gPtr->sidValues = malloc(sizeof(sidI));
	//global values
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
	/*
	for(uint8_t j=0; j<8;j++)
		{
		for(uint8_t i=0; i<8; i++)
			{
			spriteDefine(i+8*j, SPR_BASE8 + SIZE8*SIZE8 * i + SIZE8*SIZE8 * 8 * j, SIZE8, 0, 0);
			spriteSetPosition(i+8*j,32+SIZE8*i,32+SIZE8*j);
			spriteSetVisible(i+8*j,true);
			}
		}
		
	
	for(uint8_t j=0; j<4;j++)
		{
		for(uint8_t i=0; i<8; i++)
			{
			spriteDefine(i+8*j, SPR_BASE8 + SIZE8*SIZE8 * i + SIZE8*SIZE8 * 8 * j, SIZE8, 0, 0);
			spriteSetPosition(i+8*j,32+SIZE16*i,32+SIZE16*j);
			spriteSetVisible(i+8*j,true);
			}
		}
		
	for(uint8_t j=4; j<8;j++)
		{
		for(uint8_t i=0; i<8; i++)
			{
			spriteDefine(i+8*j, SPR_BASE16 + SIZE16*SIZE16 * i + SIZE16*SIZE16 * 8 * j, SIZE16, 0, 0);
			spriteSetPosition(i+8*j,32+SIZE16*i,32+SIZE16*j);
			spriteSetVisible(i+8*j,true);
			}
		}
		*/
		
	//GUI timer for polyphony
	polyTimer.units = TIMER_FRAMES;
	polyTimer.cookie = TIMER_POLY_COOKIE;
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
			if(gen->actionID >= ACT_MID && gen->actionID <= ACT_OPL) 
			{
				if(gen->actionID != gPtr->chipChoice) //only do something if different
				{				
					gPtr->chipChoice = gen->actionID; //change the chip
					hideGUI();
					clearText();
					switch(gPtr->chipChoice)
					{
						case 0:
							loadGUIMIDI();
							break;
						case 1:
							loadGUISID();
							break;
						case 2:
							loadGUIPSG();
							break;
						case 3:
							loadGUIOPL();
							break;
					}
				}

			}
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
	sid_adsr(gPtr->sidValues->ad, gPtr->sidValues->sr);
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
	sid_setPWM(gPtr->sidValues->pwdLo, gPtr->sidValues->pwdHi);
}
void checkMIDIIn()
{
}
	
void LightUp(struct lighter_UI *lit, uint8_t min, uint8_t max)
{
	for(uint8_t i=min; i<=max; i++)
	{
		updateLighter(&lit[i], SPR_BASE);
	}
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


		polyTimer.absolute = getTimerAbsolute(TIMER_FRAMES) + TIMER_POLY_DELAY;
		setTimer(&polyTimer);
		
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
	if(kernelEventData.type == kernelEvent(timer.EXPIRED))
	{
		if(kernelEventData.timer.cookie == TIMER_POLY_COOKIE)
		{
		switch(gPtr->chipChoice)
		{
			case 1: //SID
				for(uint8_t j=0;j<chipAct[2];j++)   lights[j].value = 1;			
				for(uint8_t j=chipAct[2]; j<6; j++) lights[j].value=0;
				LightUp(lights, 0,5);
				break;
			case 3: //OPL
				for(uint8_t j=6;j<chipAct[4]+6;j++) lights[j].value = 1;
				for(uint8_t j=chipAct[4]+6;j<15;j++) lights[j].value = 0;
				LightUp(lights, 6,14);
				break;
			case 2: //PSG
				for(uint8_t j=15;j<chipAct[3]+15;j++) lights[j].value = 1;
				for(uint8_t j=chipAct[3]+15;j<21;j++) lights[j].value = 0;
				LightUp(lights, 15,20);
				break;
		}
		polyTimer.absolute = getTimerAbsolute(TIMER_FRAMES) + TIMER_POLY_DELAY;
		setTimer(&polyTimer);
		}	
	}
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
