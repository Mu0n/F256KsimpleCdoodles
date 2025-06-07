#include "D:\F256\llvm-mos\code\digestMidi\.builddir\trampoline.h"

#define F256LIB_IMPLEMENTATION

#include "f256lib.h"
#include "../src/muUtils.h" //contains helper functions I often use
#include "../src/muMidi.h"  //contains basic MIDI functions I often use
#include "../src/muMidiPlay.h"  //contains basic MIDI functions I often use
#include "../src/timer0.h"  //contains basic cpu based timer0 functions I often use

#define MIDI_BASE   0x40000 //gives a nice 07ffkb until the parsed version happens
#define MIDI_PARSED 0x50000 //end of ram is 0x7FFFF, gives a nice 256kb of parsed midi


void setup(void);
void saveParsed(struct midiRecord *, struct bigParsedEventList *);



//STRUCTS

void setup()
{		//wipeBitmapBackground(0x2F,0x2F,0x2F);
	POKE(MMU_IO_CTRL, 0x00);
	// XXX GAMMA  SPRITE   TILE  | BITMAP  GRAPH  OVRLY  TEXT
	POKE(VKY_MSTR_CTRL_0, 0b00001111); //sprite,graph,overlay,text
	// XXX XXX  FON_SET FON_OVLY | MON_SLP DBL_Y  DBL_X  CLK_70
	POKE(VKY_MSTR_CTRL_1, 0b00010100); //font overlay, double height text, 320x240 at 60 Hz;
}

int main(int argc, char *argv[]) {
	
	bigParsed theBigList; //master structure to keep note of all tracks, nb of midi events, for playback
	midiRec myRecord; //keeps parsed info about the midi file, tempo, etc, for info display

	int16_t indexStart = 0; //keeps note of the byte where the MIDI string 'MThd' is, sometimes they're not at the very start of the file!
	
	setup(); //codec, timer and  graphic setup
	
	initMidiRecord(&myRecord, MIDI_BASE, MIDI_PARSED);
	initBigList(&theBigList);

	resetInstruments(false); //resets all channels to piano, for sam2695
	midiShutUp(false); //ends trailing previous notes if any, for sam2695

	if(loadSMFile("archon.mid", MIDI_BASE))
	{
		printf("\nCouldn't open the midi file");
		printf("Press space to exit.");
		hitspace();
		return 0;
	}
	printf("loaded MIDI\n");
	hitspace();

	
	indexStart = getAndAnalyzeMIDI(&myRecord, &theBigList);


	if(indexStart!=-1) //found a place to start in the loaded file, proceed to play
		{
	
		printf("First pass...\n");
		parse(indexStart,false, &myRecord, &theBigList); //count the events and prep the mem allocation for the big list of parsed midi events
		adjustOffsets(&theBigList);
		myRecord.fudge = 20.0f; //tweak this to change the tempo of the tune
		printf("Second pass...\n");
		parse(indexStart, true, &myRecord, &theBigList); //load up the actual event data in the big list of parsed midi events



	printf("track count %d\n",theBigList.trackcount);

		for(uint8_t b=0;b<theBigList.trackcount;b++)
		{
		printf("track %d event count: %d size: %d\n", b, theBigList.TrackEventList[b].eventcount, theBigList.TrackEventList[b].eventcount*MIDI_EVENT_FAR_SIZE);
		}


		printf("About to write results...\n");
		//writeDigestFile("aaa.dim",&myRecord, &theBigList);
		readDigestFile("aaa.dim",&myRecord, &theBigList);
	
		printf("wrote file...\n");
		hitspace();
	
		}
	return 0;
	}
