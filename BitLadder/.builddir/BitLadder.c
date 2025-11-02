#include "D:\F256\llvm-mos\code\BitLadder\.builddir\trampoline.h"

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

#define TIMER_PAD_DELAY 2
#define TIMER_PAD_COOKIE 29


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
 
#define MUSIC_BASE 0x50000


#define MOVE_VERT 4
#define MOVE_HORZ 4

//INCLUDES
#include "f256lib.h"
#include <stdlib.h>
#include "../src/mudispatch.h" //dispatch notes to chips
#include "../src/muUtils.h" //contains helper functions I often use
#include "../src/mumusicmap.h" //takes care of the mapping to several chips for a given tune
#include "../src/mupads.h"
#include "../src/muMidi.h"  //contains basic MIDI functions I often use
#include "../src/muMidiPlay2.h"  //contains basic MIDI functions I often use
#include "../src/mupsg.h"   //contains PSG chip functions
#include "../src/muopl3.h"   //contains OPL3 chip functions
#include "../src/musid.h"   //contains SID chip functions
#include "../src/timer0.h"   //contains SID chip functions
#include "../src/timerDefs.h"

EMBED(starspal, "../assets/stars.pal", 0x10000); //1kb
EMBED(starsbm, "../assets/stars.data",0x10400); //70kb
EMBED(planetBM, "../assets/planet.bin",0x23000); //4kb
EMBED(shipSP, "../assets/ship.bin",0x24000); //2kb
EMBED(shotBM, "../assets/shot.bin",0x24800); //64 bytes

//GLOBALS
struct timer_t planetScroll, shipExhaust, shotT, shipMT, padTimer;
struct midiRecord myRecord;

//reference tempoLUT, close to 112.5 bpm where a quarter note = 32 frames = 0,533s

bool needsToWaitExpiration = false;  //when tempo is changed, must wait to expire all pending timers before sounding again.

uint8_t currentPal = 0;
	
const char *eraLabels[4] = {"1979 TI pulse generator   ","1983 SID chip          ","1988 Yamaha FM Synth      ","1991 General MIDI          "};

	int16_t ship_X = 50;
	int16_t ship_Y = 50;
	int8_t ship_SX = 0;
	int8_t ship_SY = 0;
	uint8_t activeMove = 0;
	uint8_t controllerMode =0;
//FUNCTION PROTOTYPES
void setup(void);
void setPlanetPos(uint16_t, bool);
void zeroOutStuff();


void zeroOutStuff()
{
	if(myRecord.fileName != NULL) free(myRecord.fileName);
	myRecord.fileName = NULL;
	initMidiRecord(&myRecord, MUSIC_BASE, MUSIC_BASE);

}

void escReset()
{
	resetInstruments(true);
	shutAllSIDVoices();
	shutPSG();
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

void startTimers()
{
	
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
	
	// pads
	
	padTimer.units = TIMER_FRAMES;
	padTimer.cookie = TIMER_PAD_COOKIE;
	
	padTimer.absolute = getTimerAbsolute(TIMER_FRAMES) + TIMER_PAD_DELAY;
	setTimer(&padTimer);
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
	
	
	

	//super preparation
	escReset();
	
	//Prep SID stuff
	clearSIDRegisters();
	prepSIDinstruments();
	setMonoSID();
	
	//Prep PSG stuff
	setMonoPSG();
	
	//Prep OPL3 stuff
	opl3_initialize();
	opl3_setInstrumentAllChannels(0);
	
	//Prep MIDI stuff
	resetInstruments(false); //resets all channels to piano, for sam2695
	midiShutUp(false); //ends trailing previous notes if any, for sam2695

	midiShutAllChannels(false);
	

	//prepare track
	
	zeroOutStuff();
	
	
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
	
void checkPads(uint8_t whichPad, bool nesOrSnes)
{
	uint16_t addr;
	if(nesOrSnes==true) addr = PAD0 + (uint16_t)whichPad*(uint16_t)2;
	else addr = PAD0 + (uint16_t)(whichPad-4) * (uint16_t)2;
	
	//check pads
	if((PEEK(addr)&NES_LEFT)==0)
		{
		ship_SX = -MOVE_HORZ;
		}
	else if((PEEK(addr)&NES_RIGHT)==0)
		{
		ship_SX = MOVE_HORZ;
		}
	else ship_SX = 0;
	if((PEEK(addr)&NES_UP)==0)
		{
		ship_SY = -MOVE_VERT;
		}
	else if((PEEK(addr)&NES_DOWN)==0)
		{
		ship_SY = MOVE_VERT;
		}
	else ship_SY = 0;
}

void setChipMap()
{
	copySidInstrument(sid_instrument_defs[0], &sidInstruments[0]); //tri
	sid_setInstrument(0, 0, sidInstruments[0]);
	
	copySidInstrument(sid_instrument_defs[1], &sidInstruments[1]); //saw
	sid_setInstrument(0, 1, sidInstruments[1]);
	
	copySidInstrument(sid_instrument_defs[2], &sidInstruments[2]); //pul
	sid_setInstrument(0, 2, sidInstruments[2]);
	
	copySidInstrument(sid_instrument_defs[3], &sidInstruments[3]); //noi
	sid_setInstrument(1, 0, sidInstruments[3]);
	
	copySidInstrument(sid_instrument_defs[4], &sidInstruments[4]); //pul
	sid_setInstrument(1, 1, sidInstruments[4]);
	
	copySidInstrument(sid_instrument_defs[5], &sidInstruments[5]); //noi
	sid_setInstrument(1, 2, sidInstruments[5]);
	
	//chipXChannel[0]  = 0x02; //voice 0 | PSG
	
	
	sid_setSIDWide(1);
	chipXChannel[0]  = 0x11; //voice 1 | SID (saw)
	//chipXChannel[1]  = 0x21; //voice 1 | SID (pulse)
	//chipXChannel[9]  = 0x51; //voice 2 | SID (noi)
	
}
int main(int argc, char *argv[]) {
	int16_t scrollPlanet=320;
	uint8_t exhFlip = 0;
	int16_t shot_SX = 5;
	int16_t shot_X = 0;
	int16_t shot_Y = 0;
	uint8_t firstMIDAddrFound = 0;
	
	setup();
	textGotoXY(0,0);printf("%s",eraLabels[currentPal]);
	
	sprintf(myRecord.fileName, "%s", "midi/lvl001b.mid");
	
	loadSMFile(myRecord.fileName, MUSIC_BASE);
	detectStructure(0, &myRecord);
    initTrack(MUSIC_BASE);
	
    setChipMap();

	startTimers();
	
	for(uint16_t i=0;i<theOne.nbTracks;i++)	exhaustZeroes(i);
	resetTimer0();
	
	while(true)
		{
		kernelNextEvent();
		if(kernelEventData.type == kernelEvent(timer.EXPIRED))
			{
			switch(kernelEventData.timer.cookie)
				{
					case TIMER_PAD_COOKIE:
						if(controllerMode ==1)
							{
							pollNES();
							padPollDelayUntilReady();
							checkPads(0, true); //nes mode
							}
						else if(controllerMode==2)
							{
							pollSNES();
							padPollDelayUntilReady();
							checkPads(4, false); //snes mode
							}
	
						padTimer.absolute = getTimerAbsolute(TIMER_FRAMES) + TIMER_PAD_DELAY;
						setTimer(&padTimer);
						break;
	
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
						if(ship_X <= 320 && ship_X >= 32) ship_X += (int16_t)ship_SX;
						if(ship_Y >= 32 && ship_Y <= 248) ship_Y += (int16_t)ship_SY;
						if(ship_X < 32) ship_X = 32; if(ship_X > 320) ship_X = 320;
						if(ship_Y < 32) ship_Y = 32; if(ship_Y > 248) ship_Y = 248;
						setShipPos(ship_X,ship_Y);
						shipMT.absolute = getTimerAbsolute(TIMER_FRAMES) + TIMER_SHIP_MOVE_DELAY;
						setTimer(&shipMT);
						break;
				}
			}
		else if(kernelEventData.type == kernelEvent(key.PRESSED))
			{
			switch(kernelEventData.key.raw)
				{
				case 135: //F7 launchBeat
					break;
				case 32: //space

				currentPal++;
				if(currentPal>3) currentPal=0;
				changePals(currentPal);
				textGotoXY(0,0);printf("%s",eraLabels[currentPal]);

					break;
				case 99: //C (controller)
					controllerMode++;
					if(controllerMode > 2) controllerMode = 0;
					textGotoXY(0,3);
					switch(controllerMode)
						{
						case 0:
							textPrint("Keyb ");
							break;
						case 1:
							textPrint("NES1 ");
							break;
						case 2:
							textPrint("SNES1");
							break;
						}
					break;
				case 109: //M music select
					break;
				case 97: //A (LEFT)
					if(controllerMode == 0) ship_SX = -MOVE_HORZ;
					break;
				case 100: //D (RIGHT)
					if(controllerMode == 0) ship_SX = MOVE_HORZ;
					break;
				case 119: //W (UP)
					if(controllerMode == 0) ship_SY = -MOVE_VERT;
					break;
				case 115: //S (DOWN)
					if(controllerMode == 0) ship_SY = MOVE_VERT;
					break;
				case 120: //X (STOP)
					if(controllerMode == 0) ship_SY = ship_SX = 0;
					break;
				case 5: //ALT (SHOOT)
				if(controllerMode == 0) {
					shot_Y = ship_Y + 13;
					shot_X = ship_X + 28;
					spriteSetPosition(5,shot_X,shot_Y);
					spriteSetVisible(5,true);
					POKE((uint32_t)VKY_SP0_CTRL + (uint32_t)SPR_OFFSET*(uint32_t)5, 0x71 | (currentPal<<1));
					shotT.absolute = getTimerAbsolute(TIMER_FRAMES) + TIMER_SHOT_DELAY;
					setTimer(&shotT);
					}
					break;
	
				default:
					textGotoXY(0,10);printf("%d  ",kernelEventData.key.raw);
				}

			}
	
			if(PEEK(INT_PENDING_0)&0x10) //when the timer0 delay is up, go here
				{
				POKE(INT_PENDING_0,0x10); //clear the timer0 delay
				playMidi(); //play the next chunk of the midi file, might deal with multiple 0 delay stuff
				}
	
			if(theOne.isWaiting == false)
				{
				sniffNextMIDI(); //find next event to play, will cue up a timer0 delay
				}
			if(theOne.isMasterDone >= theOne.nbTracks)
				{
				resetTrack(MUSIC_BASE);
				}


		}
	return 0;}
