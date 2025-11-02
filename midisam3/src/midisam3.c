#define F256LIB_IMPLEMENTATION
#include "f256lib.h"
#include "../src/muUtils.h" //contains helper functions I often use
#include "../src/musid.h" //sid chip
#include "../src/muFilePicker.h"  //contains basic MIDI functions I often use
#include "../src/setup.h" //contains helper functions I often use

#include "../src/mudispatch.h" //dispatch to various chips
#include "../src/muMidi.h"  //contains basic MIDI functions I often use
#include "../src/muMidiPlay2.h"  //contains basic MIDI functions I often use
#include "../src/muTimer0Int.h"  //contains basic cpu based timer0 functions I often use
#include "../src/muTextUI.h" //text dialogs and file directory and file picking
#include "../src/mumusicmap.h" //takes care of the mapping to several chips for a given tune

#define MIDI_PARSED 0x50000 //end of ram is 0x7FFFF, gives a nice 256kb of parsed midi
#define MUSIC_BASE 0x50000

#define TIMER_FRAMES 0
#define TIMER_SECONDS 1
#define TIMER_PLAYBACK_COOKIE 0
#define TIMER_DELAY_COOKIE 1

#define DIRECTORY_X 1
#define DIRECTORY_Y 6


//STRUCTS
struct timer_t playbackTimer;
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

char currentFilePicked[32];

short optimizedMIDIShimmering() {
	
	if(kernelEventData.type == kernelEvent(key.PRESSED))
		{	
			if((kernelEventData.key.raw == 0x5B || kernelEventData.key.raw == 0x5D)  && isPaused ) //left bracket [
				{
					muteArray[presentIndex] = ~muteArray[presentIndex];
					updateMuteDisplay(presentIndex);
				}
			if(kernelEventData.key.raw == 0xB6 && isPaused) //up arrow
				{
				cycleMuteSelection(-1);	
				}
			if(kernelEventData.key.raw == 0xB7 && isPaused) //down arrow
				{
				cycleMuteSelection(1);	
				}
			if(kernelEventData.key.raw == 0x83) //F3
				{
					if(isPaused==false)
					{
						shutAllSIDVoices();
						midiShutAllChannels(false);
						//shutPSG();
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
					
					detectStructure(0, &myRecord);
					initTrack(MUSIC_BASE);
					wipeText();
					initProgress();
					displayInfo(&myRecord);
					superExtraInfo(&myRecord);		
					isPaused = false;
					
					textSetColor(0,0);textGotoXY(26,MENU_Y);textPrint("pause");
					
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
					shutAllSIDVoices();
					//shutPSG();
					isPaused = true;
					textSetColor(1,0);textGotoXY(26,MENU_Y);textPrint("pause");
					
					cycleMuteSelection(0);  //set the cursor in the first detected channel, for muting control
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
					textSetColor(1,0);textGotoXY(42,MENU_Y);textPrint("SID    ");
					textSetColor(0,0);textGotoXY(52,MENU_Y);textPrint("SAM2695");
					midiChip = false;
					}
				else
					{
					textSetColor(0,0);textGotoXY(42,MENU_Y);textPrint("SID    ");
					textSetColor(1,0);textGotoXY(52,MENU_Y);textPrint("SAM2695");
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
		} // end if key pressed
return 0;
}


void zeroOutStuff()
{
	for(uint8_t k=0;k<32;k++) currentFilePicked[k]=0;

	if(myRecord.fileName != NULL) free(myRecord.fileName);
	myRecord.fileName = NULL;
	initMidiRecord(&myRecord, MUSIC_BASE, MUSIC_BASE);
	for(uint8_t ind = 0;ind<16;ind++) 
		{
			muteArray[ind] = 0;
			presentArray[ind] = 0;
		}
}

int main(int argc, char *argv[]) {
	uint16_t i;
	
	midiChip = false;
	playbackTimer.units = TIMER_SECONDS;
	playbackTimer.cookie = TIMER_PLAYBACK_COOKIE;
	
	//wipeBitmapBackground(0x2F,0x2F,0x2F);
	POKE(MMU_IO_CTRL, 0x00);
	// XXX GAMMA  SPRITE   TILE  | BITMAP  GRAPH  OVRLY  TEXT
	POKE(VKY_MSTR_CTRL_0, 0b00000001); //sprite,graph,overlay,text
	// XXX XXX  FON_SET FON_OVLY | MON_SLP DBL_Y  DBL_X  CLK_70
	POKE(VKY_MSTR_CTRL_1, 0b00000100); //font overlay, double height text, 320x240 at 60 Hz;
	
	zeroOutStuff();
	setColors();		
	
	clearSIDRegisters();
	setMonoSID();
	
	
	//check if the midi directory is here, if not, target the root
	char *dirOpenResult = fileOpenDir("midi");
	if(dirOpenResult != NULL) 
	{
		strncpy(fpr.currentPath, "midi", MAX_PATH_LEN);
		fileCloseDir(dirOpenResult);
	}
	else strncpy(fpr.currentPath, "0:", MAX_PATH_LEN);


	uint8_t wantsQuit = filePickModal(&fpr, DIRECTORY_X, DIRECTORY_Y, "mid", "", "", "", true);
	if(wantsQuit==1) return 0;
	sprintf(myRecord.fileName, "%s%s%s", fpr.currentPath,"/", fpr.selectedFile);
	
	printf("**** %s *****",myRecord.fileName);
	hitspace();
	loadSMFile(myRecord.fileName, MUSIC_BASE);

	setColors();textGotoXY(0,25);printf("->Currently Loading file %s...",myRecord.fileName);

	wipeText();
	detectStructure(0, &myRecord);
	displayInfo(&myRecord);
	resetInstruments(false); //resets all channels to piano, for sam2695
	midiShutUp(false); //ends trailing previous notes if any, for sam2695

	midiShutAllChannels(false);
	

    initTrack(MUSIC_BASE);
	
	
	copySidInstrument(sid_instrument_defs[2], &sidInstruments[0]); //sawtooth
	sid_setInstrument(0, 0, sidInstruments[0]);
	
	copySidInstrument(sid_instrument_defs[0], &sidInstruments[1]); //sinus
	sid_setInstrument(0, 1, sidInstruments[1]);
	
	copySidInstrument(sid_instrument_defs[3], &sidInstruments[2]); //noise
	sid_setInstrument(0, 2, sidInstruments[2]);
	
	copySidInstrument(sid_instrument_defs[1], &sidInstruments[3]); //triangle
	sid_setInstrument(1, 0, sidInstruments[3]);
	
	copySidInstrument(sid_instrument_defs[1], &sidInstruments[4]); //triangle
	sid_setInstrument(1, 1, sidInstruments[4]);
	
	copySidInstrument(sid_instrument_defs[0], &sidInstruments[5]); //sinus
	sid_setInstrument(1, 2, sidInstruments[5]);
	
	sid_setSIDWide(0);
	chipXChannel[0] = 0x01; //voice 0 | sid  (sawtooth)
	chipXChannel[1] = 0x11; //voice 1 | sid  (sine)
	chipXChannel[2] = 0x31; //voice 1 | sid  (tri)
	chipXChannel[3] = 0x41; //voice 1 | sid  (tri)
	chipXChannel[4] = 0x51; //voice 1 | sid  (squ)
	chipXChannel[9] = 0x21; //voice 2 | sid  (noise)
	
	
/*
	printf("SID instrument chosen\n%02x maxVol\n%02x pwdLo %02x pwdHi\n%02x ad %02x sr\n%02x ctrl\n%02x fcfLo %02x fcfHi%02x frr\n", sidInstruments[0].maxVolume,
sidInstruments[0].pwdLo, 
sidInstruments[0].pwdHi, 
sidInstruments[0].ad, 
sidInstruments[0].sr,
sidInstruments[0].ctrl, 
sidInstruments[0].fcfLo, 
sidInstruments[0].fcfHi, 
sidInstruments[0].frr); 


	printf("SID instrument chosen\n%02x maxVol\n%02x pwdLo %02x pwdHi\n%02x ad %02x sr\n%02x ctrl\n%02x fcfLo %02x fcfHi\n%02x frr", sidInstruments[2].maxVolume,
sidInstruments[2].pwdLo, 
sidInstruments[2].pwdHi, 
sidInstruments[2].ad, 
sidInstruments[2].sr,
sidInstruments[2].ctrl, 
sidInstruments[2].fcfLo, 
sidInstruments[2].fcfHi, 
sidInstruments[2].frr); 



*/
	//chipXChannel[2] = 0x02;
//find what to do and exhaust all zero delay events at the start



	for(uint16_t i=0;i<theOne.nbTracks;i++)	exhaustZeroes(i);

	resetTimer0();			

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
				
					midiShutAllChannels(false);
					//shutPSG();
					clearSIDRegisters();
					
					destroyTrack();
					zeroOutStuff();	
					
					detectStructure(0, &myRecord);
					initTrack(MUSIC_BASE);
					textSetColor(1,0);textGotoXY(3,26);textPrint("[r]");

					}
					else 
					{
						isPaused = true;
						break; //really quit no matter what
					}
				}
				
				}
			
	midiShutAllChannels(false);
	}
	
	return 0;
}
	