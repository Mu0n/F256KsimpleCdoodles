#include "D:\F256\llvm-mos\code\vgmplayer2\.builddir\trampoline.h"

#define F256LIB_IMPLEMENTATION
#include "f256lib.h"

#include "../src/muUtils.h" //contains helper functions I often use
#include "../src/muopl3.h" //opl3 chip routines
#include "../src/muvgmplay.h" //used to play vgms
#include "../src/muFilePicker.h"  //file picker I use everywhere
#include "../src/muTimer0Int.h"  //timer0 routines with an interval of max 2/3 of a second
#include "../src/mulcd.h"  //K2 LCD
#include "../src/muMIDIin.h" //midi in keyboard
#include "../src/muMidi.h"
#include "../src/mudispatch.h"
#include "../src/textUI.h"

#include <stdlib.h>

#define LCDBIN 0x40000

void textUI(void);
void eraseLine(uint8_t);
void snoopLoop();

EMBED(pianopal, "../assets/piano.pal", 0x10000); //1kb
EMBED(keys, "../assets/piano.raw", 0x10400); //
EMBED(lcdimg, "../assets/opl3snoop.bin", 0x40000); //
midiInData gMID;
FILE *theFile;

//prototypes:
void setup(void);
int8_t dealPressedKey(void);
uint8_t findNextOne(uint8_t, bool);

// UNUSED?
/*
uint32_t convert_samples_to_timer_ticks(uint32_t samples) {
    // Timer0 max = 0xFFFFFF = 16777215 ticks = 2/3 sec
    // So: ticks_per_sample = (2/3 sec / 16777215) * vgm_rate
    // Simplified:
    return (samples * 0x66666) ;  // 667000 µs = 2/3 sec
}
*/

int8_t dealPressedKey()
{
kernelNextEvent();
if(kernelEventData.type == kernelEvent(key.PRESSED))
	{
		if(kernelEventData.key.raw == 0x83) //F3
			{
				eraseLine(0);eraseLine(1);eraseLine(39);
				return 1;
	
			}
		if(kernelEventData.key.raw == 146) //esc
			{
			opl3_quietAll();
			return -1;
			}
		if(kernelEventData.key.raw == 0x20) //space for pause
			{
			return -2;
			}
		if(kernelEventData.key.raw == 0x87) //F7
			{
			return -3;
			}
	}
return 0;
}


void setup()
{
POKE(MMU_IO_CTRL, 0x00);
// XXX GAMMA  SPRITE   TILE  | BITMAP  GRAPH  OVRLY  TEXT
POKE(VKY_MSTR_CTRL_0, 0b00001111); //sprite,graph,overlay,text
// XXX XXX  FON_SET FON_OVLY | MON_SLP DBL_Y  DBL_X  CLK_70
POKE(VKY_MSTR_CTRL_1, 0b00000100); //font overlay, double height text, 320x240 at 60 Hz;
POKE(VKY_LAYER_CTRL_0, 0b00010000); //bitmap 0 in layer 0, bitmap 1 in layer 1
POKE(0xD00D,0x00); //force black graphics background
POKE(0xD00E,0x00);
POKE(0xD00F,0x00);


POKE(MMU_IO_CTRL,1); //MMU I/O to page 1
for(uint16_t c=0;c<1023;c++)
{
	POKE(VKY_GR_CLUT_0+c, FAR_PEEK(0x10000+c)); //palette for piano
}
POKE(MMU_IO_CTRL,0); //MMU I/O to page 0
	
bitmapSetAddress(0,0x10400);
bitmapSetVisible(0,false);
initFPR();
//resetGlobals(gPtr);

POKE(MMU_IO_CTRL,0); //MMU I/O to page 0

resetMID(&gMID);
}

uint8_t findNextOne(uint8_t chan, bool isUpOrDown) //returns 0xFF if you can't find one
{
	if(isUpOrDown && chan != 0) //going up
	{
		chan--;
		while(selectableChan[chan]==0)
			{
				if(chan == 0) return 0xFF; //couldn't find any and hit rock bottom
				chan--;
				if(selectableChan[chan] == 1) return chan; //just found one
			}
		return chan;
	}
	else if(isUpOrDown == false && chan != 17)//going down
	{
		chan++;
		while(selectableChan[chan]==0)
			{
				if(chan == 17) return 0xFF; //couldn't find any and hit ceiling
				chan++;
				if(selectableChan[chan] == 1) return chan; //just found one
			}
		return chan;
	}
	return 0xFF;
	
}

void snoopLoop() //reached when in paused mode
	{
	uint8_t escape = 1;
	uint8_t indx = 0;
	playChan = 0;


	bitmapSetVisible(0,true);
	textSetColor(15,1);
	//textSetDouble(false,false);
	show_inst(playChan);
	
	opl3_quietAll();
	opl3_write(OPL_PERC, chip_VT_PERC&0xE0); //clear perc key ons but not the mode yet
	//opl3_write(OPL_CSW, chip_NOTESEL);
	opl3_set4OPS(playChan, true);
	
	
	
	while(escape)
		{
		dealMIDIIn(&gMID);
		kernelNextEvent();
	
		if(kernelEventData.type == kernelEvent(key.PRESSED))
			{
			if(kernelEventData.key.raw == 0x20) escape = 0; //spacebar
			if(kernelEventData.key.raw == 0xB6)//up arrow
				{
				uint8_t newChan = findNextOne(playChan, true);
				if(newChan != 0xFF)
					{
					textSetColor(15,0);
					show_inst(playChan); //de-highlight
					//printf("                 ");
					textSetColor(15,1);
	
					playChan = newChan;
					show_inst(playChan);
					opl3_quietAll();
					opl3_write(OPL_PERC, chip_VT_PERC&0xE0); //clear perc key ons but not the mode yet
					opl3_set4OPS(playChan, true);
					}
				}
			if(kernelEventData.key.raw == 0xB7)//down arrow
				{
				uint8_t newChan = findNextOne(playChan, false);
				if(newChan != 0xFF)
					{
					textSetColor(15,0);
					show_inst(playChan);
					//printf("                 ");
					textSetColor(15,1);
	
					playChan = newChan;
					show_inst(playChan);
					opl3_quietAll();
					opl3_write(OPL_PERC, chip_VT_PERC&0xE0); //clear perc key ons but not the mode yet
					opl3_set4OPS(playChan, true);
					}
	
				}
			}
	
		}
	
	//restoration of the status of the chip before pausing

	
	
	show_inst(playChan);printf("                 ");


	opl3_quietAll();
	opl3_write(OPL_PERC, chip_VT_PERC&0xE0); //clear perc key ons but not the mode yet
	
	opl3_write(OPL_FOE, chip_OPL3_PAIRS);

	//channels
	
	opl3_setInstrumentAllChannels(0, false);

	
	for(indx=0; indx<18; indx++)
		{
		opl3_setFrLo(opl3_instrument_defs[indx].CHAN_FRLO, indx);
	
		if(indx == 8 && chip_VT_PERC) continue;
		else if(indx == 7 && chip_VT_PERC) continue;
		else opl3_setFnum(opl3_instrument_defs[indx].CHAN_FNUM & 0xDF, indx);
		}

	
	//globals
	/*
	textGotoXY(START_X_VIS+48,START_HEIGHT_VIS_ALT);printf(" %02x ",chip_VT_PERC);
	textGotoXY(START_X_VIS+56,START_HEIGHT_VIS_ALT);printf(" %02x ",chip_NOTESEL);
	textGotoXY(START_X_VIS+52,START_HEIGHT_VIS_ALT);printf(" %02x ",chip_enable);
	*/
	bitmapSetVisible(0,false);
	}


uint8_t mainLoop()
{
bool iRT = false;
bool pauseRequested = false;
int checkKey = 0;
uint8_t exitFlag = 0;

	
while(true)//main play and input loop
	{
		int8_t playbackReturn = playback(theFile, iRT, pauseRequested);
		//textGotoXY(60,10);printf("pause %d",pauseRequested);
		if(playbackReturn == -1) //done with file
			{
			pauseRequested = false;
			break;
			}
		if(playbackReturn == -2) //pause conditions met
			{
			pauseRequested = false;
			opl3_quietAll();
			axes_info(START_HEIGHT_VIS);
			show_all_inst();
			snoopLoop();
			if(iRT==false) wipe_inst();
			}
		checkKey = dealPressedKey();
		if(checkKey == 1)
			{
			break;
			}
		if(checkKey == -1)
			{
			exitFlag = 1;
			break;
			}
		if(checkKey == -2) //pause is hit
			{
			pauseRequested = true;
			}
		if(checkKey == -3) //F7 is hit
			{
			if(iRT)
			{
				wipe_inst();
				iRT = false;
			}
			else
				{
				axes_info(START_HEIGHT_VIS);
				show_all_inst();
				iRT = true;
				}
			}
	}
	return exitFlag;
}

int main(int argc, char *argv[]) {
bool cliFile = false; //first time it loads, next times it keeps the cursor position
uint8_t exitFlag = 0;

setup();

if(argc > 1)
	{
	uint8_t i=0;
	char fileName[120];

	
	if(argv[1][0] != '-')
		{
		while(argv[1][i] != '\0')
			{
				fileName[i] = argv[1][i];
				i++;
			}
		fileName[i] = '\0';
	
		cliFile = true;
		sprintf(name, "%s", fileName);
		}
	}
	

if(hasCaseLCD())displayImage(LCDBIN);


lilpause(1);
while(exitFlag == 0)
	{
	opl3_initialize();
	instructions();
	wipe_inst();
	opl3_initialize_defs();
	

	if(cliFile == false)
	{
		if(getTheFile(name)!=0) return 0;
		theFile = load_VGM_file(name); //gets a FILE opened and ready to use
	}
	else theFile = load_VGM_file(name);
	
	checkVGMHeader(theFile);
	eraseLine(3);eraseLine(4);
	textGotoXY(0,3);textSetColor(7,0);printf("Playing: %.71s",name);
	cliFile = false;
	//info on screen
	textUI();
	comeRightTrough = true; //to kickstart it
	
	exitFlag = mainLoop();
	}

opl3_initialize();
return 0;}

