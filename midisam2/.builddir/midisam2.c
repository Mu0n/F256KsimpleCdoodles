#include "D:\F256\llvm-mos\code\midisam2\.builddir\trampoline.h"

#define F256LIB_IMPLEMENTATION
#include "f256lib.h"
#include "../src/muUtils.h" //contains helper functions I often use
#include "../src/setup.h" //contains helper functions I often use
#include "../src/muVS1053b.h" //VS1053b stuff
#include "../src/muMidi.h"  //contains basic MIDI functions I often use
#include "../src/muMidiPlay2.h"  //contains basic MIDI functions I often use
#include "../src/muFilePicker.h"  //contains basic MIDI functions I often use
#include "../src/muTimer0Int.h"  //contains basic cpu based timer0 functions I often use
#include "../src/muTextUI.h" //text dialogs and file directory and file picking
#include "../src/mulcd.h"

#define MIDI_PARSED 0x50000 //end of ram is 0x7FFFF, gives a nice 256kb of parsed midi

#define MUSIC_BASE 0x50000

#define TIMER_FRAMES 0
#define TIMER_SECONDS 1
#define TIMER_PLAYBACK_COOKIE 0
#define TIMER_DELAY_COOKIE 1
#define TIMER_SHIMMER_COOKIE 2
#define TIMER_SHIMMER_DELAY 10

#define DIRECTORY_X 1
#define DIRECTORY_Y 6

#define CLONE_TO_UART_EMWHITE 0

EMBED(cozyLCD, "../assets/cozyLCD", 0x30000);

//STRUCTS
struct timer_t playbackTimer, shimmerTimer;
struct time_t time_data;
bool isPaused = false;
bool isTrulyDone = false;

struct midiRecord myRecord;

FILE *theMIDIfile;
filePickRecord fpr;
	
bool repeatFlag = false;
//PROTOTYPES
short optimizedMIDIShimmering(void);
void zeroOutStuff(void);
void wipeShimmer(void);

char currentFilePicked[32];


//FUNCTIONS
bool isK2(void)
{
	uint8_t value = PEEK(0xD6A7) & 0x1F;
	return (value == 0x11);
}
bool K2LCD()
{
	if(isK2())
	{
	displayImage(0x30000);
	return true;
	}
	else return false;
}

void wipeShimmer()
{
	for(uint8_t i=0;i<16;i++) //channels
		{
		textGotoXY(16,8+i);
		textPrint("                                                           ");
		}
}
short optimizedMIDIShimmering() {

	if(kernelEventData.type == kernelEvent(timer.EXPIRED))
	{
		switch(kernelEventData.timer.cookie)
		{
			case TIMER_SHIMMER_COOKIE:
				shimmerTimer.absolute = getTimerAbsolute(TIMER_FRAMES)+TIMER_SHIMMER_DELAY;
				setTimer(&shimmerTimer);
				for(uint8_t i=0;i<16;i++) //channels
					{
					textSetColor(i+1,0);
					for(uint8_t j=0;j<8;j++) //number of bytes to represent on screen
						{
						textGotoXY(11+(j<<3),8+i);
						if(shimmerChanged[i][j]==false) continue; //no change, so let's continue
						shimmerChanged[i][j]=false; //will be dealt with so mark it as changed and done for next evaluation
						__putchar(shimmerBuffer[i][j]&0x01?42:32);
						__putchar(shimmerBuffer[i][j]&0x02?42:32);
						__putchar(shimmerBuffer[i][j]&0x04?42:32);
						__putchar(shimmerBuffer[i][j]&0x08?42:32);
						__putchar(shimmerBuffer[i][j]&0x10?42:32);
						__putchar(shimmerBuffer[i][j]&0x20?42:32);
						__putchar(shimmerBuffer[i][j]&0x40?42:32);
						__putchar(shimmerBuffer[i][j]&0x80?42:32);
						}
					}
				break;
		}
	}
	
	if(kernelEventData.type == kernelEvent(key.PRESSED))
		{
			if(kernelEventData.key.raw == 0x83) //F3
				{
					if(isPaused==false)
					{
						midiShutAllChannels(true);
						midiShutAllChannels(false);
						isPaused = true;
						textSetColor(1,0);textGotoXY(26,MENU_Y);textPrint("pause");
					}
					destroyTrack();
					zeroOutStuff();
					wipeText();
					uint8_t wantsQuit = filePickModal(&fpr, DIRECTORY_X, DIRECTORY_Y, "mid", "", "", "", false);
					if(wantsQuit==1) return 0;
					sprintf(myRecord.fileName, "%s%s%s", fpr.currentPath,"/", fpr.selectedFile);
					loadSMFile(myRecord.fileName, MUSIC_BASE);
					setColors();
					textGotoXY(0,25);printf("->Currently Loading file %s...",myRecord.fileName);
	
					detectStructure(0, &myRecord);
					initTrack(MUSIC_BASE);
					wipeText();
					initProgress();
					displayInfo(&myRecord);
					superExtraInfo(&myRecord);
					isPaused = false;
					textSetColor(0,0);textGotoXY(26,MENU_Y);textPrint("pause");
					shimmerTimer.absolute = getTimerAbsolute(TIMER_FRAMES)+TIMER_SHIMMER_DELAY;
					wipeShimmer();
					setTimer(&shimmerTimer);
				}
			if(kernelEventData.key.raw == 146) //esc
				{
				midiShutAllChannels(midiChip);
				isTrulyDone = true;
				return 1;
				}
			if(kernelEventData.key.raw == 32) //space
			{
				if(isPaused==false)
				{
					midiShutAllChannels(midiChip);
					isPaused = true;
					textSetColor(1,0);textGotoXY(26,MENU_Y);textPrint("pause");
				}
				else
				{
					isPaused = false;
					textSetColor(0,0);textGotoXY(26,MENU_Y);textPrint("pause");
				}
			}
			if(kernelEventData.key.raw == 129) //F1
			{
				midiShutAllChannels(midiChip);
				if(midiChip==true)
					{
					textSetColor(1,0);textGotoXY(42,MENU_Y);textPrint("SAM2695");
					textSetColor(0,0);textGotoXY(52,MENU_Y);textPrint("VS1053b");
					midiChip = false;
					}
				else
					{
					textSetColor(0,0);textGotoXY(42,MENU_Y);textPrint("SAM2695");
					textSetColor(1,0);textGotoXY(52,MENU_Y);textPrint("VS1053b");
					midiChip = true;
					}
			}
			if(kernelEventData.key.raw == 0x72) //r
				{
				repeatFlag = !repeatFlag;
				if(repeatFlag)
				{
				textSetColor(1,0);textGotoXY(3,26);textPrint("[r]");
				}
				else {
				textSetColor(0,0);textGotoXY(3,26);textPrint("[r]");
					}
				}
			if(kernelEventData.key.raw == 0x2E) //.
			{
			for(uint8_t i=0;i<16;i++)
					{
						POKE(midiChip?MIDI_FIFO_ALT:MIDI_FIFO, 0xB0 | i); // control change message
						POKE(midiChip?MIDI_FIFO_ALT:MIDI_FIFO, 0x5B); // reverb
						POKE(midiChip?MIDI_FIFO_ALT:MIDI_FIFO, 0x00);
	
						POKE(midiChip?MIDI_FIFO_ALT:MIDI_FIFO, 0xB0 | i); // control change message
						POKE(midiChip?MIDI_FIFO_ALT:MIDI_FIFO, 0x5D); // chorus
						POKE(midiChip?MIDI_FIFO_ALT:MIDI_FIFO, 0x00);
	
					}
			}
			if(kernelEventData.key.raw == 0x2C) //,
			{
			for(uint8_t i=0;i<16;i++)
					{
						POKE(midiChip?MIDI_FIFO_ALT:MIDI_FIFO, 0xB0 | i); // control change message
						POKE(midiChip?MIDI_FIFO_ALT:MIDI_FIFO, 0x07); // reverb
						POKE(midiChip?MIDI_FIFO_ALT:MIDI_FIFO, 0x7F);
	
					}
			}
		//printf("%02x",kernelEventData.key.raw);
		} // end if key pressed
return 0;
}

void machineDependent()
{
	
	uint8_t machineCheck=0;
	//VS1053b
	machineCheck=PEEK(0xD6A7)&0x3F;
	/*
	if(machineCheck == 0x22 || machineCheck == 0x11) //22 is Jr2 and 11 is K2
	{
		*/
	boostVSClock();
	initRTMIDI();
/*
	}
	
	setupSerial();
*/
}




void zeroOutStuff()
{
	for(uint8_t k=0;k<32;k++) currentFilePicked[k]=0;
	
	for(uint8_t i=0;i<16;i++)
	{
		for(uint8_t j=0;j<8;j++)
		{
			shimmerChanged[i][j]=false;
			shimmerBuffer[i][j]=0;
		}
	}
	if(myRecord.fileName != NULL) free(myRecord.fileName);
	myRecord.fileName = NULL;
	initMidiRecord(&myRecord, MUSIC_BASE, MUSIC_BASE);
}

int main(int argc, char *argv[]) {
	uint16_t i;
	openAllCODEC(); //if the VS1053b is used, this might be necessary for some board revisions
	//initVS1053MIDI();  //if the VS1053b is used
	
	K2LCD();
	
	lilpause(1);
	midiChip = false;
	playbackTimer.units = TIMER_SECONDS;
	playbackTimer.cookie = TIMER_PLAYBACK_COOKIE;
	
	shimmerTimer.units = TIMER_FRAMES;
	shimmerTimer.cookie = TIMER_SHIMMER_COOKIE;
	
	//wipeBitmapBackground(0x2F,0x2F,0x2F);
	POKE(MMU_IO_CTRL, 0x00);
	// XXX GAMMA  SPRITE   TILE  | BITMAP  GRAPH  OVRLY  TEXT
	POKE(VKY_MSTR_CTRL_0, 0b00000001); //sprite,graph,overlay,text
	// XXX XXX  FON_SET FON_OVLY | MON_SLP DBL_Y  DBL_X  CLK_70
	POKE(VKY_MSTR_CTRL_1, 0b00000100); //font overlay, double height text, 320x240 at 60 Hz;
	

	machineDependent();
	zeroOutStuff();
	setColors();
	
	/*
	uint8_t v =0;
	for(;;)
	{
		printf("%c %02x\n", argv[1][v],argv[1][v]);
		v++;
		hitspace();
	}
	*/
	
	//check if the midi directory is here, if not, target the root
	char *dirOpenResult = fileOpenDir("media/midi");
	if(dirOpenResult != NULL)
	{
		strncpy(fpr.currentPath, "media/midi", MAX_PATH_LEN);
		fileCloseDir(dirOpenResult);
	}
	else strncpy(fpr.currentPath, "0:", MAX_PATH_LEN);
	

	lilpause(1);
	if(argc > 1)
	{
		i=0;
		if(argv[1][0] == '-')
			{
			uint8_t wantsQuit = filePickModal(&fpr, DIRECTORY_X, DIRECTORY_Y, "mid", "", "", "", true);
			if(wantsQuit==1) return 0;
			sprintf(myRecord.fileName, "%s%s%s", fpr.currentPath,"/", fpr.selectedFile);
			loadSMFile(myRecord.fileName, MUSIC_BASE);
			}
		else
			{
			while(argv[1][i] != '\0')
				{
					myRecord.fileName[i] = argv[1][i];
					i++;
				}

			myRecord.fileName[i] = '\0';
	
			sprintf(fpr.selectedFile, "%s", myRecord.fileName);
	
			if(loadSMFile(argv[1], MUSIC_BASE))
				{
				printf("\nCouldn't open %s\n",argv[1]);
				printf("Press space to exit.");
				hitspace();
				return 0;
				}
			}
	
	lilpause(1);
	}
	else
	{
	uint8_t wantsQuit = filePickModal(&fpr, DIRECTORY_X, DIRECTORY_Y, "mid", "", "", "", true);
	if(wantsQuit==1) return 0;
	sprintf(myRecord.fileName, "%s%s%s", fpr.currentPath,"/", fpr.selectedFile);
	loadSMFile(myRecord.fileName, MUSIC_BASE);

	lilpause(1);
	}
	setColors();textGotoXY(0,25);printf("->Currently Loading file %s...",myRecord.fileName);

	wipeText();
	detectStructure(0, &myRecord);
	displayInfo(&myRecord);
	resetInstruments(false); //resets all channels to piano, for sam2695
	midiShutUp(false); //ends trailing previous notes if any, for sam2695
	midiShutUp(true); //ends trailing previous notes if any, for sam2695

	midiShutAllChannels(true);
	midiShutAllChannels(false);
	
    initTrack(MUSIC_BASE);
	
//find what to do and exhaust all zero delay events at the start
	for(uint16_t i=0;i<theOne.nbTracks;i++)	exhaustZeroes(i);

	resetTimer0();
	shimmerTimer.absolute = getTimerAbsolute(TIMER_FRAMES)+TIMER_SHIMMER_DELAY;
	setTimer(&shimmerTimer);
	while(!isTrulyDone)
	{
			wipeStatus();
			superExtraInfo(&myRecord);
	
			initProgress();
			setColors();
			for(;;)
				{
				kernelNextEvent();
				if(optimizedMIDIShimmering()) break;
				if(!isPaused)
					{

					if(PEEK(INT_PENDING_0)&0x10) //when the timer0 delay is up, go here
						{
						POKE(INT_PENDING_0,0x10); //clear the timer0 delay
						playMidi(); //play the next chunk of the midi file, might deal with multiple 0 delay stuff
						}
	
					if(theOne.isWaiting == false)
						{
						sniffNextMIDI(); //find next event to play, will cue up a timer0 delay
						}
	
					}
				if(theOne.isMasterDone >= theOne.nbTracks)
				{
					if(repeatFlag)
					{
	
					midiShutAllChannels(true);
					midiShutAllChannels(false);
	
					destroyTrack();
					zeroOutStuff();
	
					detectStructure(0, &myRecord);
					initTrack(MUSIC_BASE);
					textSetColor(1,0);textGotoXY(3,26);textPrint("[r]");

	
					shimmerTimer.absolute = getTimerAbsolute(TIMER_FRAMES)+TIMER_SHIMMER_DELAY;
					setTimer(&shimmerTimer);
					}
					else
					{
						isPaused = true;
						break; //really quit no matter what
					}
				}
	
				}
	
	midiShutAllChannels(true);
	midiShutAllChannels(false);
	}
	return 0;
}
