#include "D:\F256\llvm-mos\code\FusionDrive\.builddir\trampoline.h"


//DEFINES
#define F256LIB_IMPLEMENTATION

#define NES_CTRL    0xD880
#define NES_CTRL_TRIG 0x80
#define NES_CTRL_MODE_NES 0x05
#define NES_CTRL_MODE_SNES 0x04

#define NES_STAT    0xD880
#define NES_PAD0    0xD884
#define NES_PAD0_A      0x7F
#define NES_PAD0_B      0xBF
#define NES_PAD0_SELECT 0xDF
#define NES_PAD0_START  0xEF
#define NES_PAD0_UP     0xF7
#define NES_PAD0_DOWN   0xFB
#define NES_PAD0_LEFT   0xFD
#define NES_PAD0_RIGHT  0xFE


#define PAL_BASE    0x10000
#define BITMAP_BASE      0x20000
#define SPR_L0 0x10400
#define SPR_R0 0x10800
#define TIMER_FRAMES 0
#define TIMER_SECONDS 1
#define TIMER_CAR_COOKIE 0
#define TIMER_LANE_COOKIE 1
#define TIMER_CAR_DELAY 1
#define TIMER_LANE_DELAY 3

#define VKY_SP0_CTRL  0xD900 //Sprite #0’s control register
#define VKY_SP0_AD_L  0xD901 // Sprite #0’s pixel data address register
#define VKY_SP0_AD_M     0xD902
#define VKY_SP0_AD_H     0xD903
#define VKY_SP0_POS_X_L  0xD904 // Sprite #0’s X position register
#define VKY_SP0_POS_X_H  0xD905
#define VKY_SP0_POS_Y_L  0xD906 // Sprite #0’s Y position register
#define VKY_SP0_POS_Y_H  0xD907
#define SPR_CLUT_COLORS       32


#define T0_PEND     0xD660
#define T0_MASK     0xD66C

#define T0_CTR      0xD650 //master control register for timer0, write.b0=ticks b1=reset b2=set to last value of VAL b3=set count up, clear count down
#define T0_STAT     0xD650 //master control register for timer0, read bit0 set = reached target val

#define CTR_INTEN   0x80  //present only for timer1? or timer0 as well?
#define CTR_ENABLE  0x01
#define CTR_CLEAR   0x02
#define CTR_LOAD    0x04
#define CTR_UPDOWN  0x08

#define T0_VAL_L    0xD651 //current 24 bit value of the timer
#define T0_VAL_M    0xD652
#define T0_VAL_H    0xD653

#define T0_CMP_CTR  0xD654 //b0: t0 returns 0 on reaching target. b1: CMP = last value written to T0_VAL
#define T0_CMP_L    0xD655 //24 bit target value for comparison
#define T0_CMP_M    0xD656
#define T0_CMP_H    0xD657

#define T0_CMP_CTR_RECLEAR 0x01
#define T0_CMP_CTR_RELOAD  0x02

#define MIDI_BASE   0x38000 //gives a nice 128kb until the parsed version happens
#define MIDI_PARSED 0x58000 //end of ram is 0x7FFFF
//offsets from MIDI_PARSED
#define AME_DELTA 0
#define AME_BYTECOUNT 4
#define AME_MSG 5

#define MIDI_EVENT_FAR_SIZE 8


//INCLUDES
#include "f256lib.h"
#include <stdlib.h>
#include "../src/muUtils.h" //contains helper functions I often use
#include "../src/muMidi.h"  //contains basic MIDI functions I often use
#include "../src/midispec.c"

EMBED(palback, "../assets/Urban4.data.pal", 0x10000);
EMBED(carL, "../assets/car.spr",0x10400);
EMBED(carR, "../assets/carr.spr",0x10800);
EMBED(backbmp, "../assets/Urban4.data", 0x20000);
EMBED(midifile, "../assets/continen.mid", 0x38000);

//GLOBALS
struct timer_t carTimer, laneTimer;

bigParsed theBigList;
FILE *theMIDIfile;
uint16_t gFormat = 0; //holds the format type for the info provided after loading 0=single track, 1=multitrack
uint16_t trackcount = 0; // #number of tracks detected
uint32_t tick = 0; // #microsecond per tick
uint32_t gFileSize; //keeps track of how many bytes in the file
bool readyForNextNote = 1; //interrupt-led flag to signal the loop it's ready for the next MIDI event
int16_t tempAddr; //used to compute addresses for pokes
double fudge = 21; //fudge factor when calculating time delays
uint16_t *parsers; //indices for the various type 1 tracks during playback
uint32_t *targetParse; //will pick the default 0x20000 if the file doesn't load
uint8_t nn=4, dd=2, cc=24, bb=8; //nn numerator of time signature, dd= denom. cc=nb of midi clocks in metro click. bb = nb of 32nd notes in a beat
uint8_t gSineIndex=0;
uint16_t carX=160, carY=220;

int16_t SIN[] = {
       0,    1,    3,    4,    6,    7,    9,   10,
      12,   14,   15,   17,   18,   20,   21,   23,
      24,   25,   27,   28,   30,   31,   32,   34,
      35,   36,   38,   39,   40,   41,   42,   44,
      45,   46,   47,   48,   49,   50,   51,   52,
      53,   54,   54,   55,   56,   57,   57,   58,
      59,   59,   60,   60,   61,   61,   62,   62,
      62,   63,   63,   63,   63,   63,   63,   63,
      64,   63,   63,   63,   63,   63,   63,   63,
      62,   62,   62,   61,   61,   60,   60,   59,
      59,   58,   57,   57,   56,   55,   54,   54,
      53,   52,   51,   50,   49,   48,   47,   46,
      45,   44,   42,   41,   40,   39,   38,   36,
      35,   34,   32,   31,   30,   28,   27,   25,
      24,   23,   21,   20,   18,   17,   15,   14,
      12,   10,    9,    7,    6,    4,    3,    1,
       0,   -1,   -3,   -4,   -6,   -7,   -9,  -10,
     -12,  -14,  -15,  -17,  -18,  -20,  -21,  -23,
     -24,  -25,  -27,  -28,  -30,  -31,  -32,  -34,
     -35,  -36,  -38,  -39,  -40,  -41,  -42,  -44,
     -45,  -46,  -47,  -48,  -49,  -50,  -51,  -52,
     -53,  -54,  -54,  -55,  -56,  -57,  -57,  -58,
     -59,  -59,  -60,  -60,  -61,  -61,  -62,  -62,
     -62,  -63,  -63,  -63,  -63,  -63,  -63,  -63,
     -64,  -63,  -63,  -63,  -63,  -63,  -63,  -63,
     -62,  -62,  -62,  -61,  -61,  -60,  -60,  -59,
     -59,  -58,  -57,  -57,  -56,  -55,  -54,  -54,
     -53,  -52,  -51,  -50,  -49,  -48,  -47,  -46,
     -45,  -44,  -42,  -41,  -40,  -39,  -38,  -36,
     -35,  -34,  -32,  -31,  -30,  -28,  -27,  -25,
     -24,  -23,  -21,  -20,  -18,  -17,  -15,  -14,
     -12,  -10,   -9,   -7,   -6,   -4,   -3,   -1,
};

//FUNCTION PROTOTYPES
void setup(void);

void sendAME(aMEPtr midiEvent); //sends a MIDI event message, either a 2-byte or 3-byte one
int16_t findPositionOfHeader(void);  //this opens a .mid file and ignores everything until 'MThd' is encountered
void detectStructure(uint16_t startIndex); //checks the tempo, number of tracks, etc
int16_t getAndAnalyzeMIDI(void); //high level function that directs the reading and parsing of the MIDI file
int8_t parse(uint16_t startIndex, bool wantCmds);
uint32_t getTotalLeft(void);
void playmidi(void);
void adjustOffsets(void);
void resetTimer0(void);
void setTimer0(uint8_t, uint8_t, uint8_t);
uint32_t readTimer0(void);
uint8_t isTimer0Done(void);

void updateCarPos(uint8_t *value);
void setCarPos(uint16_t, uint16_t);
void hitspace(void);
void swapColors(uint8_t, uint8_t);

//sends a MIDI event message, either a 2-byte or 3-byte one
void sendAME(aMEPtr midiEvent)
	{
	POKE(MIDI_FIFO, midiEvent->msgToSend[0]);
	POKE(MIDI_FIFO, midiEvent->msgToSend[1]);
	if(midiEvent->bytecount == 3) POKE(MIDI_FIFO, midiEvent->msgToSend[2]);
	}
//this opens a .mid file and ignores everything until 'MThd' is encountered
int16_t findPositionOfHeader(void)
	{
	char targetSequence[] = "MThd";
    char *position;
    int16_t thePosition = 0;
	char buffer[400];
	uint16_t i=0;
	
	for(i=0;i<400;i++) buffer[i] = FAR_PEEK(MIDI_BASE + i);
	
    position = strstr(buffer, targetSequence);
	
	if(position != NULL)
		{
		thePosition = (int16_t)(position - buffer);
		return thePosition;
		}
	return -1;
	}
//checks the tempo, number of tracks, etc
void detectStructure(uint16_t startIndex)
	{
    uint32_t trackLength = 0; //size in bytes of the current track
    uint32_t i = startIndex; // #main array parsing index
    uint32_t j=0;
    uint16_t currentTrack=0; //index for the current track

    i+=4;

	i+=4; //skip size

    gFormat =
     	           (uint16_t) (FAR_PEEK(MIDI_BASE+i+1))
    			  |(uint16_t) (FAR_PEEK(MIDI_BASE+i)<<8)
    			  ;
    i+=2;
 
    trackcount =
     	           (uint16_t)(FAR_PEEK(MIDI_BASE+i+1))
    			  |(uint16_t)((FAR_PEEK(MIDI_BASE+i)<<8))
    			  ;
    i+=2;

    i+=2;//skip tdiv

	currentTrack=0;
 
	 theBigList.hasBeenUsed = true;
     theBigList.trackcount = trackcount;
     theBigList.TrackEventList = (aTOEPtr) malloc((sizeof(aTOE))*theBigList.trackcount);
	
	parsers = (uint16_t *) malloc(sizeof(uint16_t) * trackcount);
	
    while(currentTrack < trackcount)
    	{
			parsers[currentTrack] = 0;
	    	currentTrack++;
	    	i+=4; //skips the MTrk string
	
	    	trackLength =  (((uint32_t)(FAR_PEEK(MIDI_BASE+i)))<<24)
		             | (((uint32_t)(FAR_PEEK(MIDI_BASE+i+1)))<<16)
					 | (((uint32_t)(FAR_PEEK(MIDI_BASE+i+2)))<<8)
					 |  ((uint32_t)(FAR_PEEK(MIDI_BASE+i+3)));
	        i+=4;
	
	        i+=trackLength;
	
    	} //end of parsing all tracks

     for(j=0;j<theBigList.trackcount;j++)
        {
	
        theBigList.TrackEventList[j].trackno = j;
        theBigList.TrackEventList[j].eventcount = 0;
		theBigList.TrackEventList[j].baseOffset = 0;
		}
	}
//high level function that directs the reading and parsing of the MIDI file
int16_t getAndAnalyzeMIDI(void)
	{
	uint8_t result=0;
	
	int16_t indexToStart=0; //MThd should be at position 0, but it might not, so we'll find it
	
	
	//result = loadSMFile("human2.mid"); //actual process of asking for a midi file through std mac dialog
	
	
	if(result ==1) return -1;
	
	indexToStart = findPositionOfHeader(); //find the start index of 'MThd'
	
	if(indexToStart == -1)
		{
		return -1;
		}
	detectStructure(indexToStart); //parse it a first time to get the format type and nb of tracks
	
	return indexToStart;
	}

//assuming a byte buffer that starts with MThd, read all tracks and produce a structure of aMIDIEvent arrays
// wantCmds is when it's ready to record the relevant commands in an array
int8_t parse(uint16_t startIndex, bool wantCmds)
	{
    uint32_t trackLength = 0; //size in bytes of the current track
    uint16_t tickPerBeat=48; //time per division in ticks; 48 is the default midi value
    uint32_t i = startIndex; // #main array parsing index
    uint16_t currentTrack=0; //index for the current track
	uint32_t tempCalc=0;
    uint8_t last_cmd = 0x00;
    uint32_t currentI;
	uint16_t interestingIndex=0;
    uint32_t nValue, nValue2, nValue3, nValue4, timeDelta;
    uint8_t status_byte = 0x00, extra_byte = 0x00, extra_byte2 = 0x00;
    uint8_t meta_byte = 0x00;
    uint32_t data_byte = 0x00, data_byte2= 0x00, data_byte3 = 0x00, data_byte4= 0x00;
	uint32_t usPerBeat=500000; //this is read off of a meta event 0xFF 0x51, 3 bytes long
    bool lastCmdPreserver = false;
	
	uint32_t whereTo=0; //where to write individual midi events in far memory
	
    //first pass will find the number of events to keep in the TOE (table of elements)
    //and initialize the myParsedEventList, an array of TOE arrays

    //second pass will actually record the midi events into the appropriate TOE for each track

    i+=4; //skips 'MThd' midi file header 4 character id string

	i+=4; //skip size
 
    gFormat =
     	           (uint16_t) (FAR_PEEK(MIDI_BASE+i+1))
    			  |(uint16_t) (FAR_PEEK(MIDI_BASE+i)<<8)
    			  ;
    i+=2;

    trackcount =
     	           (uint16_t)(FAR_PEEK(MIDI_BASE+i+1))
    			  |(uint16_t)((FAR_PEEK(MIDI_BASE+i)<<8))
    			  ;
    i+=2;

 
    tickPerBeat = (uint16_t)(
     	           (uint16_t)(FAR_PEEK(MIDI_BASE+i+1))
    			  |(uint16_t)((FAR_PEEK(MIDI_BASE+i)<<8))
    			  );
    i+=2;
    currentTrack=0;
 
	
    while(currentTrack < trackcount)
    	{
	
		i+=4; //skip 'MTrk' header 4 character string
	
		trackLength =  (((uint32_t)(FAR_PEEK(MIDI_BASE+i)))<<24)
		             | (((uint32_t)(FAR_PEEK(MIDI_BASE+i+1)))<<16)
					 | (((uint32_t)(FAR_PEEK(MIDI_BASE+i+2)))<<8)
					 |  ((uint32_t)(FAR_PEEK(MIDI_BASE+i+3)));
        i+=4; //skip track length in bytes
 
 
    	last_cmd = 0x00;
    	currentI = i;
		interestingIndex=0; //keeps track of how many midi events we're detecting that we want to keep
    	while(i < (trackLength + currentI))
    		{

    		nValue = 0x00000000;
    		nValue2 = 0x00000000;
    		nValue3 = 0x00000000;
    		nValue4 = 0x00000000;
    		data_byte = 0x00000000;
    		status_byte = 0x00;
	
			nValue = (uint32_t)FAR_PEEK(MIDI_BASE+i);
			i++;
			if(nValue & 0x00000080)
				{
				nValue &= 0x0000007F;
				nValue <<= 7;
				nValue2 = (uint32_t)FAR_PEEK(MIDI_BASE+i);
				i++;
				if(nValue2 & 0x00000080)
					{
					nValue2 &= 0x0000007F;
					nValue2 <<= 7;
					nValue <<= 7;
					nValue3 = (uint32_t)FAR_PEEK(MIDI_BASE+i);
					i++;
					if(nValue3 & 0x00000080)
						{
						nValue3 &= 0x0000007F;
						nValue3 <<= 7;
						nValue2 <<= 7;
						nValue <<= 7;
						nValue4 = (uint32_t)FAR_PEEK(MIDI_BASE+i);
						i++;
						} //end of getting to nValue4
					} //end of getting to nValue3
				} //end of getting to nValue2
    		timeDelta = nValue | nValue2 | nValue3 | nValue4;
	

    		//status byte or MIDI message reading
    		status_byte = FAR_PEEK(MIDI_BASE+i);
			extra_byte  = FAR_PEEK(MIDI_BASE+i+1); //be ready for 2 bytes midi events
			extra_byte2 = FAR_PEEK(MIDI_BASE+i+2); //be ready for 3 bytes midi events
			i++;
	
			lastCmdPreserver = false;
			//first, check for run-on commands that don't repeat the status_byte
			if(status_byte < 0x80)
				{
				i--;//go back 1 spot so it can read the data properly
				status_byte = last_cmd;
				extra_byte  = FAR_PEEK(MIDI_BASE+i); //recycle the byte to its proper destination
				extra_byte2 = FAR_PEEK(MIDI_BASE+i+1); //redo: be ready for 3 bytes midi events
	
				}
			//second, deal with MIDI meta-event commands that start with 0xFF
			if(status_byte == 0xFF)
				{
				meta_byte = (uint8_t)FAR_PEEK(MIDI_BASE+i);
				i++;
				if(meta_byte == MetaSequence)
					{
					i+=2;
					}
				else if(meta_byte == MetaText)
					{
					data_byte = (uint8_t)FAR_PEEK(MIDI_BASE+i); //length of text
					i+=(uint16_t)data_byte + 1;
					}
				else if(meta_byte == MetaCopyright)
					{
					data_byte = (uint8_t)FAR_PEEK(MIDI_BASE+i); //length of text
					i+=(uint16_t)data_byte + 1;
					}
				else if(meta_byte == MetaTrackName)
					{
					data_byte = (uint8_t)FAR_PEEK(MIDI_BASE+i); //length of text
					i+=(uint16_t)data_byte + 1;
					}
				else if(meta_byte == MetaInstrumentName)
					{
					data_byte = (uint8_t)FAR_PEEK(MIDI_BASE+i); //length of text
					i+=(uint16_t)data_byte + 1;
					}
				else if(meta_byte == MetaLyrics)
					{
					data_byte = (uint8_t)FAR_PEEK(MIDI_BASE+i); //length of text
					i+=(uint16_t)data_byte + 1;
					}
				else if(meta_byte == MetaMarker)
					{
					data_byte = (uint8_t)FAR_PEEK(MIDI_BASE+i); //length of text
					i+=(uint16_t)data_byte + 1;
					}
				else if(meta_byte == MetaCuePoint)
					{
					data_byte = (uint8_t)FAR_PEEK(MIDI_BASE+i); //length of text
					i+=(uint16_t)data_byte + 1;
					}
				else if(meta_byte == MetaChannelPrefix)
					{
					i+=2;
					}
				else if(meta_byte == MetaChangePort)
			    	{
			    	i+=2;
			    	}
				else if(meta_byte == MetaEndOfTrack)
					{
					i++;
					continue;
					}
				else if(meta_byte == MetaSetTempo)
					{
					data_byte = (uint8_t)FAR_PEEK(MIDI_BASE+i);
					i++;
					data_byte2 = (uint8_t)FAR_PEEK(MIDI_BASE+i);
					i++;
					data_byte3 = (uint8_t)FAR_PEEK(MIDI_BASE+i);
					i++;
					data_byte4 = (uint8_t)FAR_PEEK(MIDI_BASE+i);
					i++;
	
					usPerBeat = ( ((uint32_t)data_byte2)<<16 ) |
								   ( ((uint32_t)data_byte3)<<8  ) |
								     ((uint32_t)data_byte4);
	
					//if you divide usPerBeat by tickPerBeat,
					//you get the duration in microseconds per tick, ready to be multiplied	by the events' deltaTimes to get delays in us

					tick = (uint32_t)usPerBeat/((uint32_t)tickPerBeat);
					tick = (uint32_t)((double)tick * (double)fudge); //convert to the units of timer0

	
					}
				else if(meta_byte == MetaSMPTEOffset)
					{
					i+=6;
					}
				else if(meta_byte == MetaTimeSignature)
					{
					i++; //skip, it should be a constant 0x04 here
					nn = (uint8_t)FAR_PEEK(MIDI_BASE+i);
					i++;
					dd = (uint8_t)FAR_PEEK(MIDI_BASE+i);
					i++;
					cc = (uint8_t)FAR_PEEK(MIDI_BASE+i);
					i++;
					bb = (uint8_t)FAR_PEEK(MIDI_BASE+i);
					i++;
					//printf("nn=%d dd=%d cc=%d bb=%d",nn,dd,cc,bb);
					}
				else if(meta_byte == MetaKeySignature)
					{
					i+=3;
					}
				else if(meta_byte == MetaSequencerSpecific)
					{
					continue;
					}
	
				//else printf("\nUnrecognized MetaEvent %d %d\n",status_byte,meta_byte);
				} //end if for meta events
			//Third, deal with regular MIDI commands
	
			//MIDI commands with only 1 data byte
			//Program change   0xC_
			//Channel Pressure 0xD_
			else if(status_byte >= 0xC0 && status_byte <= 0xDF)
				{
	
	
				if(wantCmds == false) //merely counting here
					{
					theBigList.TrackEventList[currentTrack].eventcount++;
					}
				if(wantCmds) //prep the MIDI event
					{
	
					whereTo  = (uint32_t)(theBigList.TrackEventList[currentTrack].baseOffset);
					whereTo += (uint32_t)( (uint32_t)interestingIndex * (uint32_t) MIDI_EVENT_FAR_SIZE);
	
					tempCalc = tick * timeDelta;
					FAR_POKEW((uint32_t) MIDI_PARSED + (uint32_t) whereTo    			, (tempCalc & 0xFFFF0000)  >>16)  ;
					FAR_POKEW((uint32_t) MIDI_PARSED + (uint32_t) whereTo + (uint32_t) 2,  tempCalc & 0x0000FFFF ) ;
	
					FAR_POKE((uint32_t) MIDI_PARSED + (uint32_t) whereTo + (uint32_t) AME_BYTECOUNT, 0x02);
					FAR_POKE((uint32_t) MIDI_PARSED + (uint32_t) whereTo + (uint32_t) AME_MSG, status_byte);
					FAR_POKE((uint32_t) MIDI_PARSED + (uint32_t) whereTo + (uint32_t) AME_MSG+(uint32_t) 1, extra_byte);
					FAR_POKE((uint32_t) MIDI_PARSED + (uint32_t) whereTo + (uint32_t) AME_MSG+(uint32_t) 2, 0x00); //not needed but meh
	
					interestingIndex++;
					}
				i++; //advance the index either way
				} //end of prg ch or chan pres

	
			//MIDI commands with 2 data bytes
			// Note off 0x8_
			// Note on  0x9_
			// Polyphonic Key Pressure 0xA_ (aftertouch)
			// Control Change 0xB_
			// (0xC_ and 0xD_ have been taken care of above already)
			// Pitch Bend 0xE_
			else if((status_byte >= 0x80 && status_byte <= 0xBF) || (status_byte >= 0xE0 && status_byte <= 0xEF))
				{
				if(wantCmds == false) //merely counting here
					{
					theBigList.TrackEventList[currentTrack].eventcount++;
					}
				if(wantCmds) //prep the MIDI event
					{
					if((status_byte & 0xF0) == 0x90 && extra_byte2==0x00)
					{
						//if(currentTrack == 4) printf("before status: %02x %02x %02x\n",status_byte, extra_byte, extra_byte2);
						status_byte = status_byte & 0x8F;	//sometimes note offs are note ons with 0 velocity, quirk of some midi sequencers
						extra_byte2 = 0x7F;
						//printf("status: %02x %02x %02x\n",status_byte, extra_byte, extra_byte2);
	
						//if(currentTrack == 4) printf("after status: %02x %02x %02x\n",status_byte, extra_byte, extra_byte2);
						lastCmdPreserver = true;
						//if(currentTrack == 4) hitspace();
					}
					whereTo =  (uint32_t)(theBigList.TrackEventList[currentTrack].baseOffset);
					whereTo += (uint32_t)( (uint32_t)interestingIndex * (uint32_t) MIDI_EVENT_FAR_SIZE);
	
					tempCalc = tick * timeDelta;
					FAR_POKEW((uint32_t) MIDI_PARSED + (uint32_t) whereTo    			, (tempCalc & 0xFFFF0000)  >>16)  ;
					FAR_POKEW((uint32_t) MIDI_PARSED + (uint32_t) whereTo + (uint32_t) 2,  tempCalc & 0x0000FFFF ) ;
	
					FAR_POKE((uint32_t) MIDI_PARSED + (uint32_t) whereTo + (uint32_t) AME_BYTECOUNT, 0x03);
					FAR_POKE((uint32_t) MIDI_PARSED + (uint32_t) whereTo + (uint32_t) AME_MSG, status_byte);
					FAR_POKE((uint32_t) MIDI_PARSED + (uint32_t) whereTo + (uint32_t) AME_MSG+(uint32_t) 1, extra_byte);
					FAR_POKE((uint32_t) MIDI_PARSED + (uint32_t) whereTo + (uint32_t) AME_MSG+(uint32_t) 2, extra_byte2);
	
					interestingIndex++;
					}
				i+=2; //advance the index either way
				}// end of 3-data-byter events
	

    		else
    			{
    			//printf("\n ---Unrecognized event sb= %02x",status_byte);
    			}
    		last_cmd = status_byte;
    		if(lastCmdPreserver) last_cmd = (status_byte & 0x0F) | 0x90; //revert to note one if a note on 0 velocity was detected.
	
    		} //end of parsing a track
			currentTrack++;
    	} //end of parsing all tracks
	
		return 0;
     }	 //end of parse function
 
	
//gets a count of the total MIDI events that are relevant and left to play
uint32_t getTotalLeft(void)
	{
	uint32_t sum=0;
	uint16_t i=0;

	for(i=0; i<theBigList.trackcount; i++)
		{
		sum+=(theBigList).TrackEventList[i].eventcount;
		}
	return sum;
	}
	
void playmidi(void) //non-destructive version
{
	uint16_t i;
	uint16_t lowestTrack=0;
	uint16_t localTotalLeft=0;
	uint32_t lowestTimeFound = 0xFFFFFFFF;
	uint32_t whereTo, whereToLowest; //in far memory, keeps adress of event to send
	uint16_t trackcount;
	uint32_t delta; //used to compare times
	uint32_t overFlow;
	bool exitFlag = false;
	uint32_t *soundBeholders; //keeps a scan of the next delta in line for each track
	
	aME msgGo;
	trackcount = theBigList.trackcount;
	
	
	soundBeholders=(uint32_t*)malloc(trackcount * sizeof(uint32_t));
	
	localTotalLeft = getTotalLeft();
	
	while(localTotalLeft > 0 && !exitFlag)
	{
	for(i=0;i<trackcount;i++) //pick their first deltas to start things
	{
		if(theBigList.TrackEventList[i].eventcount == 0) //empty track, avoid picking the delta of the 1st event of next track
		{
			soundBeholders[i]=0;
			continue;
		}
		whereTo  = (uint32_t)(theBigList.TrackEventList[i].baseOffset);
		soundBeholders[i] =  (((uint32_t)((FAR_PEEKW((uint32_t) MIDI_PARSED + (uint32_t) whereTo))) << 16)
							|  (uint32_t) (FAR_PEEKW((uint32_t) MIDI_PARSED + (uint32_t) whereTo + (uint32_t) 2)));
	}
	
	
	while(localTotalLeft > 0 && !exitFlag)
		{
		lowestTimeFound = 0xFFFFFFFF; //make it easy to find lower than this
	
	
		//For loop attempt to find the most pressing event with the lowest time delta to go
		for(i=0; i<trackcount; i++)
			{
			if(parsers[i] >= (theBigList.TrackEventList[i].eventcount)) continue; //this track is exhausted, go to next
	
			delta = soundBeholders[i];

			if(delta == 0)
			{
				lowestTimeFound = 0;
				lowestTrack = i;
	
				whereToLowest  = (uint32_t)(theBigList.TrackEventList[i].baseOffset);
				whereToLowest += (uint32_t)((uint32_t) parsers[i] *(uint32_t)  MIDI_EVENT_FAR_SIZE);
	
				break; //will not find better than 0 = immediately
			}
			//is it the lowest found yet?
			if(delta < lowestTimeFound)
				{
				lowestTimeFound = delta;
				lowestTrack = i; //new record in this track, keep looking for better ones
	
				whereToLowest  = (uint32_t)(theBigList.TrackEventList[i].baseOffset);
				whereToLowest += (uint32_t)((uint32_t) parsers[i] *(uint32_t)  MIDI_EVENT_FAR_SIZE);
				}
			}  //end of the for loop for most imminent event
//hitspace();
		//Do the event
			msgGo.deltaToGo = lowestTimeFound;
			msgGo.bytecount =    FAR_PEEK((uint32_t) MIDI_PARSED + (uint32_t) whereToLowest + (uint32_t) AME_BYTECOUNT);
			msgGo.msgToSend[0] = FAR_PEEK((uint32_t) MIDI_PARSED + (uint32_t) whereToLowest + (uint32_t) AME_MSG);
			msgGo.msgToSend[1] = FAR_PEEK((uint32_t) MIDI_PARSED + (uint32_t) whereToLowest + (uint32_t) AME_MSG+(uint32_t) 1);
			msgGo.msgToSend[2] = FAR_PEEK((uint32_t) MIDI_PARSED + (uint32_t) whereToLowest + (uint32_t) AME_MSG+(uint32_t) 2);
	
		if(lowestTimeFound==0) //do these 0 delay events right away, no need to involve a Time Manager
			{
			sendAME(&msgGo);
			updateCarPos(&gSineIndex);
			}
		else
			{ //for all the rest which have a time delay
	
	
	
				overFlow = lowestTimeFound;
				while(overFlow > 0x00FFFFFF) //0x00FFFFFF is the max value of the timer0 we can do
				{
	
					setTimer0(0xFF,0xFF,0xFF);
					while(isTimer0Done()==0) updateCarPos(&gSineIndex); //delay up to maximum of 0x00FFFFFF = 2/3rds of a second
					POKE(T0_PEND,0x10); //clear timer0 at 0x10
					overFlow = overFlow - 0x00FFFFFF; //reduce the max value one by one until there is a remainder smaller than the max amount
				}
				//do the last delay that's under 2/3rds of a second
				setTimer0((uint8_t)(overFlow&0x000000FF),
					  (uint8_t)((overFlow&0x0000FF00)>>8),
				      (uint8_t)((overFlow&0x00FF0000)>>16));
				while(isTimer0Done()==0) updateCarPos(&gSineIndex);
					;
                POKE(T0_PEND,0x10); //clear timer0 at 0x10
			    sendAME(&msgGo);
			}
	
		//Advance the marker for the track that just did something
		parsers[lowestTrack]+=1;
		whereToLowest  = (uint32_t)(theBigList.TrackEventList[lowestTrack].baseOffset);
		whereToLowest += (uint32_t)((uint32_t) parsers[lowestTrack] *(uint32_t)  MIDI_EVENT_FAR_SIZE);
		//replenish the new delta here
		soundBeholders[lowestTrack] =    (((uint32_t)((FAR_PEEKW((uint32_t) MIDI_PARSED + (uint32_t) whereToLowest))) << 16)
										| (uint32_t) (FAR_PEEKW((uint32_t) MIDI_PARSED + (uint32_t) whereToLowest + (uint32_t) 2)));
		for(i=0;i<trackcount;i++)
		{
			if(parsers[i] >= (theBigList.TrackEventList[i].eventcount)) continue;//that track was already exhausted
			if(i==lowestTrack) continue; //don't mess with the track that just acted
			soundBeholders[i] -= lowestTimeFound;
		}
	
		localTotalLeft--;
		}
	}
	}
	
//adjustOffsets function
void adjustOffsets()
{
	uint16_t i=0,k=0, currentEventCount=0;
	
	for(i=0;i<theBigList.trackcount;i++)
	{
		theBigList.TrackEventList[i].baseOffset = (uint32_t)0;
		for(k=0;k<i;k++) //do this for all tracks before it
			{
			currentEventCount=theBigList.TrackEventList[k].eventcount;
			theBigList.TrackEventList[i].baseOffset += (uint32_t)(currentEventCount*MIDI_EVENT_FAR_SIZE ); //skip to a previous track
			}
	}
}

void setTimer0(uint8_t low, uint8_t mid, uint8_t hi)
{
	resetTimer0();
	POKE(T0_CMP_CTR, T0_CMP_CTR_RECLEAR); //when the target is reached, bring it back to value 0x000000
	POKE(T0_CMP_L,low);POKE(T0_CMP_M,mid);POKE(T0_CMP_H,hi); //inject the compare value as max value
}

void resetTimer0()
{
	POKE(T0_CTR, CTR_CLEAR);
	POKE(T0_CTR, CTR_UPDOWN | CTR_ENABLE);
	POKE(T0_PEND,0x10); //clear pending timer0
}
uint32_t readTimer0()
{
	return (uint32_t)((PEEK(T0_VAL_H)))<<16 | (uint32_t)((PEEK(T0_VAL_M)))<<8 | (uint32_t)((PEEK(T0_VAL_L)));
}
uint8_t isTimer0Done()
{
	return PEEK(T0_PEND)&0x10;
}
	
void hitspace()
{
	bool exitFlag = false;
	
	while(exitFlag == false)
	{
			kernelNextEvent();
			if(kernelEventData.type == kernelEvent(key.PRESSED))
			{
				switch(kernelEventData.key.raw)
				{
					case 148: //enter
					case 32: //space
						exitFlag = true;
						break;
				}
			}
	}
}

void setCarPos(uint16_t x, uint16_t y)
{
	spriteSetPosition(0,x,y);
	spriteSetPosition(1,x+32,y);
}
void updateCarPos(uint8_t *value)
{
	POKE(NES_CTRL,NES_CTRL_TRIG);
	kernelNextEvent();
	if(kernelEventData.type == kernelEvent(timer.EXPIRED))
	{
		switch(kernelEventData.timer.cookie)
		{
			case TIMER_CAR_COOKIE:
			/*
				setCarPos(160+SIN[(*value)], 220);
				(*value)+=6;
	
				*/
				carTimer.absolute = getTimerAbsolute(TIMER_FRAMES) + TIMER_CAR_DELAY;
				setTimer(&carTimer);
				if((NES_STAT & 0x40) == 0x40)
				{
					if(PEEK(NES_PAD0)==NES_PAD0_LEFT)
					{
						carX-= 3;
						setCarPos(carX, carY);
					}
					else if(PEEK(NES_PAD0) == NES_PAD0_RIGHT)
					{
						carX += 3;
						setCarPos(carX, carY);
					}
				}
				break;
			case TIMER_LANE_COOKIE:
				swapColors(112,146);
				laneTimer.absolute = getTimerAbsolute(TIMER_FRAMES) + TIMER_LANE_DELAY;
				setTimer(&laneTimer);
				break;
	
		}
	}
}

void swapColors(uint8_t c1, uint8_t c2)
{
	uint8_t tempR=0, tempG=0, tempB=0;
	
	POKE(MMU_IO_CTRL,1);
	tempR = PEEK(VKY_GR_CLUT_0+4*c2);
	tempG = PEEK(VKY_GR_CLUT_0+4*c2+1);
	tempB = PEEK(VKY_GR_CLUT_0+4*c2+2);
	POKE(VKY_GR_CLUT_0+4*c2,    PEEK(VKY_GR_CLUT_0+4*c1));
	POKE(VKY_GR_CLUT_0+4*c2+1,	PEEK(VKY_GR_CLUT_0+4*c1+1));
	POKE(VKY_GR_CLUT_0+4*c2+2,	PEEK(VKY_GR_CLUT_0+4*c1+2));
	
	POKE(VKY_GR_CLUT_0+4*c1,    tempR);
	POKE(VKY_GR_CLUT_0+4*c1+1,	tempG);
	POKE(VKY_GR_CLUT_0+4*c1+2,	tempB);
	
	POKE(MMU_IO_CTRL,0);
}
void setup()
{
	uint32_t c;
	
	POKE(MMU_IO_CTRL, 0x00);
	// XXX GAMMA  SPRITE   TILE  | BITMAP  GRAPH  OVRLY  TEXT
	POKE(VKY_MSTR_CTRL_0, 0b00101100); //sprite,graph,overlay,text
	// XXX XXX  FON_SET FON_OVLY | MON_SLP DBL_Y  DBL_X  CLK_70
	POKE(VKY_MSTR_CTRL_1, 0b00010100); //font overlay, double height text, 320x240 at 60 Hz;
	POKE(VKY_LAYER_CTRL_0, 0b00000001); //bitmap 1 in layer 0, bitmap 0 in layer 1
	POKE(VKY_LAYER_CTRL_1, 0b00000010); //bitmap 2 in layer 2
	POKE(0xD00D,0x00); //force black graphics background
	POKE(0xD00E,0x00);
	POKE(0xD00F,0x00);
	POKE(MMU_IO_CTRL,1);  //MMU I/O to page 1
	// Set up CLUT0.
	for(c=0;c<1023;c++)
	{
		POKE(VKY_GR_CLUT_0+c, FAR_PEEK(PAL_BASE+c));
	}

	
	POKE(MMU_IO_CTRL,0);
	bitmapSetActive(0);
	bitmapSetAddress(0,BITMAP_BASE);
	bitmapSetCLUT(0);
	
	bitmapSetVisible(0,true);
	bitmapSetVisible(1,false);
	bitmapSetVisible(2,false);
	
	spriteDefine(0,SPR_L0,32,0,0);
	spriteSetVisible(0,true);
	spriteDefine(1,SPR_R0,32,0,0);
	spriteSetVisible(1,true);
	
	setCarPos(160,220);

	//set NES_CTRL
	POKE(NES_CTRL,NES_CTRL_MODE_NES);

	/*
#define VKY_SP0_CTRL  	 0xD900 //Sprite #0’s control register
#define VKY_SP0_AD_L 	 0xD901 // Sprite #0’s pixel data address register
#define VKY_SP0_AD_M     0xD902
#define VKY_SP0_AD_H     0xD903
#define VKY_SP0_POS_X_L  0xD904 // Sprite #0’s X position register
#define VKY_SP0_POS_X_H  0xD905
#define VKY_SP0_POS_Y_L  0xD906 // Sprite #0’s Y position register
#define VKY_SP0_POS_Y_H  0xD907
*/

	POKE(MMU_IO_CTRL, 0x00);
	
}



/*
*/
	
void prepTimers()
	{
	carTimer.units = TIMER_FRAMES;
	carTimer.absolute = getTimerAbsolute(TIMER_FRAMES) + TIMER_CAR_DELAY;
	carTimer.cookie = TIMER_CAR_COOKIE;
	setTimer(&carTimer);

	laneTimer.units = TIMER_FRAMES;
	laneTimer.absolute = getTimerAbsolute(TIMER_FRAMES) + TIMER_LANE_DELAY;
	laneTimer.cookie = TIMER_LANE_COOKIE;
	setTimer(&laneTimer);
	}
	
	
int main(int argc, char *argv[]) {
	bool exitFlag=false;
	//uint32_t frame[5] = {SPR_F1, SPR_F2, SPR_F3, SPR_F4, SPR_F5};
	//uint8_t index=0;
	uint16_t i=0;
	uint16_t indexStart=0;
	bool isDone = false;
	
	setup();
	midiShutUp(false);
	prepTimers();
	//Prep the big digested data structure for when the MIDI file is analyzed and ready to play
	theBigList.hasBeenUsed = false;
	theBigList.trackcount = 0;
	theBigList.TrackEventList = (aTOEPtr)NULL;
	
	setTimer0(0,0,0);
	indexStart = getAndAnalyzeMIDI();
	while(!exitFlag)
		{
		parse(indexStart,false); //count the events and prep the mem allocation for the big list
	
		adjustOffsets();

		parse(indexStart,true); //load up the actual event data in the big list
	
		while(!isDone)
			{
			playmidi();
	
			//reset the parsers
			for(i=0;i < theBigList.trackcount;i++)
				{
				parsers[i] = 0;
				}
			}
	

/*
			index++;
			if(index>4) index=0;
			POKEA(VKY_SP0_AD_L, frame[index]);
*/
	
		}
	return 0;}
