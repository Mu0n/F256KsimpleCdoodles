#define F256LIB_IMPLEMENTATION
#include "f256lib.h"
#include "../src/muUtils.h" //contains helper functions I often use
#include "../src/setup.h" //contains helper functions I often use
#include "../src/muVS1053b.h" //VS1053b stuff
#include "../src/muMidi.h"  //contains basic MIDI functions I often use
#include "../src/muMidiPlay2.h"  //contains basic MIDI functions I often use
#include "../src/muTimer0Int.h"  //contains basic cpu based timer0 functions I often use

#define MIDI_BASE   0x20000 //gives a nice 07ffkb until the parsed version happens
#define MIDI_PARSED 0x50000 //end of ram is 0x7FFFF, gives a nice 256kb of parsed midi

#define MUSIC_BASE 0x50000

#define TIMER_FRAMES 0
#define TIMER_SECONDS 1
#define TIMER_PLAYBACK_COOKIE 0
#define TIMER_DELAY_COOKIE 1
#define TIMER_SHIMMER_COOKIE 2
#define TIMER_SHIMMER_DELAY 3

#define CLONE_TO_UART_EMWHITE 0

//STRUCTS
struct timer_t playbackTimer, shimmerTimer;
struct time_t time_data;
bool isPaused = false;

bool repeatFlag = false;
bool shimmerChanged[16][8];
uint8_t shimmerBuffer[16][8];
bool midiChip = false; //true = vs1053b, false=sam2695
//PROTOTYPES
void displayInfo(struct midiRecord *);
void extraInfo(struct midiRecord *,struct bigParsedEventList *);
void superExtraInfo(struct midiRecord *);
void midiPlaybackShimmering(uint8_t, uint8_t, bool);
void updateInstrumentDisplay(uint8_t, uint8_t);
short optimizedMIDIShimmering(void);

uint16_t disp[10] = {0xD6B3, 0xD6B4, 0xD6AD, 
				     0xD6AE, 0xD6AF, 0xD6AA, 
					 0xD6AB, 0xD6AC, 0xD6A7, 0xD6A9};
//FUNCTIONS

void displayInfo(struct midiRecord *rec) {
	uint8_t i=0;
	wipeStatus();
	
	textGotoXY(1,1);
	textSetColor(1,0);textPrint("Filename: ");
	textSetColor(0,0);printf("%s",rec->fileName);
	textGotoXY(1,2);
	textSetColor(1,0);textPrint("Type ");textSetColor(0,0);printf(" %d ", rec->format);
	textSetColor(1,0);textPrint("MIDI file with ");
	textSetColor(0,0);printf("%d ",rec->trackcount);
	textSetColor(1,0);(rec->trackcount)>1?textPrint("tracks"):textPrint("track");
	textSetColor(0,0);textGotoXY(1,7);textPrint("CH Instrument");
	for(i=0;i<16;i++)
	{
		textGotoXY(1,8+i);printf("%02d ",i);
	}
	textGotoXY(4,8+9);textSetColor(10,0);textPrint("Percussion");
	
	textGotoXY(0,25);printf(" ->Currently parsing file %s...",rec->fileName);
}

void extraInfo(struct midiRecord *rec,struct bigParsedEventList *list) {
	
	wipeStatus();
	textGotoXY(1,3); 
	textSetColor(0,0);printf("%lu ", getTotalLeft(list));
	textSetColor(1,0);textPrint("total event count");
	textGotoXY(40,2); 
	textSetColor(1,0);textPrint("Tempo: ");
	textSetColor(0,0);printf("%d ",rec->bpm);
	textSetColor(1,0);textPrint("bpm");
	textGotoXY(40,3);
	textSetColor(1,0);textPrint("Time Signature ");
	textSetColor(0,0);printf("%d:%d",rec->nn,1<<(rec->dd));
	textGotoXY(0,25);printf("  ->Preparing for playback...                   ");
}
void superExtraInfo(struct midiRecord *rec) {
	uint16_t temp;
	
	temp=(uint32_t)((rec->totalDuration)/125000);
	temp=(uint32_t)((((double)temp))/((double)(rec->fudge)));
	rec->totalSec = temp;
	textGotoXY(68,5); printf("%d:%02d",temp/60,temp % 60);
	textGotoXY(1,24);textPrint("[ESC]: quit    [SPACE]:  pause    [F1] Toggle MIDI Output:  ");
	textSetColor(1,0);textPrint("SAM2695");
    textSetColor(0,0);textPrint("   VS1053b");
	textGotoXY(1,25);textPrint("midiplayer v2.0 by Mu0n, July 2025");
	
	textGotoXY(1,26);textPrint("  [r] toggle repeat when done");
}
void updateInstrumentDisplay(uint8_t chan, uint8_t pgr) {
	uint8_t i=0,j=0;
	textGotoXY(4,8+chan);textSetColor(chan+1,0);
	if(chan==9)
		{
			textPrint("Percussion");
			return;
		}
	for(i=0;i<12;i++)
	{

		if(midi_instruments[pgr][i]=='\0') 
		{
			for(j=i;j<12;j++) textPrint(" ");
			break;
		}
		printf("%c",midi_instruments[pgr][i]);
	}
}

short optimizedMIDIShimmering() {
	short i,j;
	
	kernelNextEvent();
	if(kernelEventData.type == kernelEvent(timer.EXPIRED))
	{
		switch(kernelEventData.timer.cookie)
		{
			case TIMER_SHIMMER_COOKIE:
				shimmerTimer.absolute = getTimerAbsolute(TIMER_FRAMES)+TIMER_SHIMMER_DELAY;
				setTimer(&shimmerTimer);
				for(i=0;i<16;i++) //channels
					{
					textSetColor(i+1,0);
					for(j=0;j<8;j++) //number of bytes to represent on screen
						{	
						textGotoXY(11+(j<<3),8+i);
						if(shimmerChanged[i][j]==false) continue; //no change, so let's continue
						shimmerChanged[i][j]=false; //will be dealt with so mark it as changed and done for next evaluation
						__putchar(shimmerBuffer[i][j]&0x80?42:32);
						__putchar(shimmerBuffer[i][j]&0x40?42:32);
						__putchar(shimmerBuffer[i][j]&0x20?42:32);
						__putchar(shimmerBuffer[i][j]&0x10?42:32);
						__putchar(shimmerBuffer[i][j]&0x08?42:32);
						__putchar(shimmerBuffer[i][j]&0x04?42:32);
						__putchar(shimmerBuffer[i][j]&0x02?42:32);
						__putchar(shimmerBuffer[i][j]&0x01?42:32);
						}
					}
				break;
		}
	}
	if(kernelEventData.type == kernelEvent(key.PRESSED))
		{
			if(kernelEventData.key.raw == 146) //esc
				{
				midiShutAllChannels(midiChip);
				return 1;
				}
			if(kernelEventData.key.raw == 32) //space
			{
				if(isPaused==false)
				{
					midiShutAllChannels(midiChip);
					isPaused = true;
					textSetColor(1,0);textGotoXY(26,24);textPrint("pause");
				}	
				else
				{
					isPaused = false;
					textSetColor(0,0);textGotoXY(26,24);textPrint("pause");				
				}
			}
			if(kernelEventData.key.raw == 129) //F1
			{
				midiShutAllChannels(midiChip);
				if(midiChip==true)
					{
					textSetColor(1,0);textGotoXY(61,24);textPrint("SAM2695");
					textSetColor(0,0);textGotoXY(71,24);textPrint("VS1053b");
					midiChip = false;
					}
				else
					{
					textSetColor(0,0);textGotoXY(61,24);textPrint("SAM2695");
					textSetColor(1,0);textGotoXY(71,24);textPrint("VS1053b");
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
		//printf("%02x",kernelEventData.key.raw);
		} // end if key pressed
return 0;
}

int main(int argc, char *argv[]) {

	uint8_t exitCode = 0;
	bool isDone = false; //to know when to exit the main loop; done playing
	int16_t indexStart = 0; //keeps note of the byte where the MIDI string 'MThd' is, sometimes they're not at the very start of the file!
	uint8_t i=0,j=0;
	uint8_t machineCheck=0;
	openAllCODEC(); //if the VS1053b is used, this might be necessary for some board revisions	
	//initVS1053MIDI();  //if the VS1053b is used
    struct midiRecord myRecord;
	
	
	//wipeBitmapBackground(0x2F,0x2F,0x2F);
	POKE(MMU_IO_CTRL, 0x00);
	// XXX GAMMA  SPRITE   TILE  | BITMAP  GRAPH  OVRLY  TEXT
	POKE(VKY_MSTR_CTRL_0, 0b00000001); //sprite,graph,overlay,text
	// XXX XXX  FON_SET FON_OVLY | MON_SLP DBL_Y  DBL_X  CLK_70
	POKE(VKY_MSTR_CTRL_1, 0b00000100); //font overlay, double height text, 320x240 at 60 Hz;
	
	for(i=0;i<16;i++)
	{
		for(j=0;j<8;j++)
		{
			shimmerChanged[i][j]=false;
			shimmerBuffer[i][j]=0;
		}
	}
	
	//VS1053b
	machineCheck=PEEK(0xD6A7)&0x3F;
	if(machineCheck == 0x22 || machineCheck == 0x11) //22 is Jr2 and 11 is K2
	{
	boostVSClock();
	initRTMIDI();
	}

	if(CLONE_TO_UART_EMWHITE==1)
	{
		//init speed of UART for EMWhite
		POKE(0xD633,128);
		POKE(0xD630,50);
		POKE(0xD631,0);
		POKE(0xD632,0);
		POKE(0xD633,3);
		
		POKE(0xD632,0b11100111);
		POKE(0xD631,0);
	}

	
	initMidiRecord(&myRecord, MUSIC_BASE, MUSIC_BASE);
	if(argc > 1)
	{
		
		i=0;
		while(argv[1][i] != '\0')
		{
			myRecord.fileName[i] = argv[1][i];
		i++;
		}		

		myRecord.fileName[i] = '\0';
		
	}
	else
	{
		printf("Invalid file name. Launch as /- midisam.pgz midifile.mid\n");
		printf("Press space to exit.");
		hitspace();
		return 0;
	}
	setColors();
	textGotoXY(0,25);printf("->Currently Loading file %s...",argv[1]);
	
	
	detectStructure(0, &myRecord);
	displayInfo(&myRecord);
	resetInstruments(false); //resets all channels to piano, for sam2695
	midiShutUp(false); //ends trailing previous notes if any, for sam2695
	
	playbackTimer.units = TIMER_SECONDS;
	playbackTimer.cookie = TIMER_PLAYBACK_COOKIE;
		
	shimmerTimer.units = TIMER_FRAMES;
	shimmerTimer.cookie = TIMER_SHIMMER_COOKIE;

	
	if(loadSMFile(argv[1], MUSIC_BASE))
	{
		printf("\nCouldn't open %s\n",argv[1]);
		printf("Press space to exit.");
		hitspace();
		return 0;
	}

    initTrack(MUSIC_BASE);
//find what to do and exhaust all zero delay events at the start
	exhaustZeroes();
	
	POKE(0xD6A0,0xE3);
	for(uint8_t i=0;i<3;i++)
	{
		POKE(0xD6A7+i,0);
		POKE(0xD6AA+i,0);
		POKE(0xD6AD+i,0);
		POKE(0xD6B3+i,0);
	}
	
	setTimer0(0);
	if(indexStart!=-1) //found a place to start in the loaded file, proceed to play
		{
		//extraInfo(&myRecord,&theBigList);
		wipeStatus();
		superExtraInfo(&myRecord);		
		
		while(!isDone)
			{
			initProgress();
			
			for(;;)
			{
			optimizedMIDIShimmering();
			if(PEEK(INT_PENDING_0)&0x10) //when the timer0 delay is up, go here
				{
				POKE(INT_PENDING_0,0x10); //clear the timer0 delay
				playMidi(); //play the next chunk of the midi file, might deal with multiple 0 delay stuff
				}
			sniffNextMIDI(); //find next event to play, will cue up a timer0 delay
			}
			if(exitCode == 1) isDone = true; //really quit no matter what
			if(repeatFlag == false) isDone=true;
			}
		}	
	midiShutAllChannels(true);
	midiShutAllChannels(false);
	return 0;
	}
	