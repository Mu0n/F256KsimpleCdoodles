
//DEFINES
#define F256LIB_IMPLEMENTATION

#define NES_CTRL    0xD880

#define NES_CTRL_TRIG 0b10000000
#define NES_STAT_DONE 0b01000000
//D880 settings
//        7  6  5  4 |  3  2   1  0
// NES_TRIG XX XX XX | XX MODE XX NES_EN
#define NES_CTRL_MODE_NES  0b00000001
#define NES_CTRL_MODE_SNES 0b00000101

#define NES_STAT    0xD880
#define NES_PAD0    0xD884
#define NES_PAD1    0xD886
#define NES_A      0x7F
#define NES_B      0xBF
#define NES_SELECT 0xDF
#define NES_START  0xEF
#define NES_UP     0xF7
#define NES_DOWN   0xFB
#define NES_LEFT   0xFD
#define NES_RIGHT  0xFE

#define ABS(a) ((a) < 0 ? -(a) : (a)) 

#define TIMER_SECONDS 1
#define TIMER_CAR_COOKIE 0
#define TIMER_LANE_COOKIE 1
#define TIMER_CAR_FORCE_COOKIE 2
#define TIMER_CAR_DELAY 1
#define TIMER_LANE_DELAY 3
#define TIMER_CAR_FORCE_DELAY 3

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


#define PAL_BASE    0x10000
#define BITMAP_BASE      0x20000
#define SPR_FRONT_L 0x10400
#define SPR_FRONT_R 0x10800
#define SPR_MIDDLE_L 0x10C00
#define SPR_MIDDLE_R 0x11000
#define SPR_BACK_L 0x011400
#define SPR_BACK_R 0x011800   
#define SPR_YEL_L 0x011C00    
#define SPR_YEL_R 0x012000    
#define SPR_YEL_FAR1 0x012400
#define SPR_YEL_FAR2 0x012800
#define SPR_YEL_FAR3 0x012C00             
#define TIMER_FRAMES 0

#define TOP_SPEED 4
#define TOP_MIN_X 42
#define TOP_MAX_X 278
#define TOP_FORCE 2
#define ENMY_ENTRY_Y 170

//INCLUDES
#include "f256lib.h"
#include <stdlib.h>
#include "../src/muUtils.h" //contains helper functions I often use
#include "../src/muMidi.h"  //contains basic MIDI functions I often use
#include "../src/timer0.h"  //contains timer0 routines

EMBED(palback, "../assets/Urban4.data.pal", 0x10000); //1kb
EMBED(carF, "../assets/carfront.bin",0x10400); //2kb
EMBED(carM, "../assets/carmid.bin",0x10C00);   //2kb
EMBED(carB, "../assets/carback.bin",0x11400); //2kb
EMBED(yellow, "../assets/yellow.bin",0x11C00); //5kb
//next would be at 0x13000
EMBED(backbmp, "../assets/Urban4.data", 0x20000);
EMBED(midifile, "../assets/continen.mid", 0x38000);

//GLOBALS
struct timer_t carTimer, laneTimer, carForceTimer;

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

//GLOBALS
typedef struct sprStatus
{
	uint8_t s; //sprite index in Vicky
	int16_t x,y,z; //position
	bool rightOrLeft; //last facing
	bool isDashing;
	uint32_t addr; //base address
	uint8_t frame; //frame into view
	uint16_t sx, sy; //speed
	int8_t vx, vy; //velocity
	int8_t ax, ay; //acceleration
	struct timer_t timer; //animation timer;
	uint8_t cookie; //cookie for timer
	uint8_t state; //which state: 0 idle, 1 walk right, 2 walk left, etc
	uint8_t *minIndexForState; //minimum index for given state
	uint8_t *maxIndexForState; //maximum index for given state
} sprStatus;

struct sprStatus car_back_L, car_back_R, car_mid_L, car_mid_R, car_front_L, car_front_R,
                 car_yel_L, car_yel_R;


//FUNCTION PROTOTYPES
void setup(void);

void sendAME(aMEPtr); //sends a MIDI event message, either a 2-byte or 3-byte one
int16_t findPositionOfHeader(void);  //this opens a .mid file and ignores everything until 'MThd' is encountered	
void detectStructure(uint16_t); //checks the tempo, number of tracks, etc
int16_t getAndAnalyzeMIDI(void); //high level function that directs the reading and parsing of the MIDI file  
int8_t parse(uint16_t, bool);
void playmidi(void);
void adjustOffsets(void);

void mySetCar(uint8_t, uint32_t, uint8_t, uint8_t, uint8_t, uint8_t, uint16_t, uint16_t, uint8_t, bool, struct sprStatus *);
void updateCarPos(uint8_t *);
void setCarPos(int16_t, int16_t);
void swapColors(uint8_t, uint8_t);
void setEnemyPos(struct sprStatus*);

//sends a MIDI event message, either a 2-byte or 3-byte one
void sendAME(aMEPtr midiEvent)
	{
	POKE(MIDI_FIFO_ALT, midiEvent->msgToSend[0]);
	POKE(MIDI_FIFO_ALT, midiEvent->msgToSend[1]);
	//POKE(MIDI_FIFO, midiEvent->msgToSend[0]);
	//POKE(MIDI_FIFO, midiEvent->msgToSend[1]);
	if(midiEvent->bytecount == 3) 
	{
		POKE(MIDI_FIFO_ALT, midiEvent->msgToSend[2]);
		//POKE(MIDI_FIFO, midiEvent->msgToSend[2]);
	}
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
	
	localTotalLeft = getTotalLeft(&theBigList);
	
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


void setCarPos(int16_t x, int16_t y)
{
	int16_t midx, frontx;
	
	midx  = x-(x-160)/18;
	frontx= x-(x-160)/12;
	spriteSetPosition(0,x,y);
	spriteSetPosition(1,x+32,y);
	spriteSetPosition(2,midx,y);
	spriteSetPosition(3,midx+32,y);
	spriteSetPosition(4,frontx,y);
	spriteSetPosition(5,frontx+32,y);
}

void setEnemyPos(struct sprStatus *theSpr)
{
	uint32_t tempAddr=0;
	
	switch(theSpr->z)
	{
		case 3:
		    tempAddr = theSpr->addr + (uint32_t)4096;
			spriteDefine(theSpr->s, tempAddr , 32, 0, 1);
			spriteSetVisible(theSpr->s+1,false);
			break;
		case 2:
			tempAddr =  theSpr->addr + (uint32_t)3072;
			spriteDefine(theSpr->s, tempAddr, 32, 0, 1);
			spriteSetVisible(theSpr->s+1,false);
			break;
		case 1:
			tempAddr = theSpr->addr + (uint32_t)2048;
			spriteDefine(theSpr->s, tempAddr, 32, 0, 1);
			spriteSetVisible(theSpr->s+1,false);
			break;
		case 0:
			tempAddr =  theSpr->addr;
			spriteDefine(theSpr->s,	tempAddr , 32, 0, 0);
			spriteDefine(theSpr->s+1, tempAddr + (uint32_t)1024, 32, 0, 0);
			spriteSetVisible(theSpr->s+1,true);
			break;
	}
	spriteSetVisible(theSpr->s,true);
	textGotoXY(0,15);
	
	printf("ex:%d ey:%d ez:%d        ",theSpr->x, theSpr->y, theSpr->z);
	spriteSetPosition(theSpr->s,theSpr->x  - (theSpr->z==0?16:0),theSpr->y);
	spriteSetPosition(theSpr->s+1,theSpr->x+32 - (theSpr->z==0?16:0),theSpr->y);
	theSpr->y += theSpr->sy;
	if(theSpr->y>=220) 
	{
		spriteDefine(theSpr->s,tempAddr,32,0,0);
		spriteDefine(theSpr->s+1,tempAddr + (uint32_t)1024,32,0,0);
	spriteSetVisible(theSpr->s,true);
	spriteSetVisible(theSpr->s+1,true);
	}
	else
	{
		spriteDefine(theSpr->s,tempAddr,32,0,1);
	}
	if(theSpr->y>240) theSpr->y = ENMY_ENTRY_Y;
	theSpr->z = 3 - (theSpr->y - ENMY_ENTRY_Y)/12;
	if(theSpr->z < 0) theSpr->z = 0;
	
}
void updateCarPos(uint8_t *value)
{
	POKE(NES_CTRL, NES_CTRL_MODE_NES | NES_CTRL_TRIG);
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
				while((PEEK(NES_STAT) & NES_STAT_DONE) != NES_STAT_DONE)
					;
				if(PEEK(NES_PAD1)==NES_LEFT)
				{
					car_yel_L.x --;
				}
				else if(PEEK(NES_PAD1) == NES_RIGHT)
				{
					car_yel_L.x ++;
				}
				if(PEEK(NES_PAD1)==NES_DOWN)
				{
					car_yel_L.z--;
				}
				else if(PEEK(NES_PAD1)==NES_UP)
				{
					car_yel_L.z++;
				}
				if(PEEK(NES_PAD0)==NES_LEFT)
				{
					car_back_L.ax = -TOP_FORCE;
				}
				else if(PEEK(NES_PAD0) == NES_RIGHT)
				{
					car_back_L.ax = TOP_FORCE;
				}
				else 
				{
					car_back_L.ax = 0;
				}
				car_back_L.x += car_back_L.vx;
				if(car_back_L.x < TOP_MIN_X) 
				{
					car_back_L.vx = 0;
					car_back_L.x = TOP_MIN_X;
				}
				if(car_back_L.x > TOP_MAX_X) 
				{
					car_back_L.vx = 0;
					car_back_L.x = TOP_MAX_X;
				}

				setCarPos(car_back_L.x, car_back_L.y);
				textGotoXY(0,10);
				printf("x:%d vx:%d ax:%d              ", car_back_L.x, car_back_L.vx, car_back_L.ax);
				break;
			case TIMER_LANE_COOKIE:
				swapColors(112,146);
				laneTimer.absolute = getTimerAbsolute(TIMER_FRAMES) + TIMER_LANE_DELAY;
				setTimer(&laneTimer);
				break;
			case TIMER_CAR_FORCE_COOKIE:
				setEnemyPos(&car_yel_L);
				if(car_back_L.ax == 0) //no longer pushing to the sides; start the deceleration process due to friction
				{
					if(car_back_L.vx < 0 && car_back_L.vx != 0) car_back_L.vx++;
					if(car_back_L.vx > 0 && car_back_L.vx != 0) car_back_L.vx--;
				}
				else car_back_L.vx += car_back_L.ax; //top speed limitation, for both negative and positive speed orientation
				
				if(car_back_L.vx > TOP_SPEED) car_back_L.vx = TOP_SPEED; //speed limiters
				if(car_back_L.vx < -TOP_SPEED) car_back_L.vx = -TOP_SPEED;
				//if(car_back_L.vx == 0) car_back_L.ax = 0;
				
				carForceTimer.absolute = getTimerAbsolute(TIMER_FRAMES)+TIMER_CAR_FORCE_DELAY;
				setTimer(&carForceTimer);
				break;
				
		}
	}
	//clear the trig
	POKE(NES_CTRL, NES_CTRL_MODE_NES);	
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
	POKE(VKY_MSTR_CTRL_0, 0b00101111); //sprite,graph,overlay,text
	// XXX XXX  FON_SET FON_OVLY | MON_SLP DBL_Y  DBL_X  CLK_70
	POKE(VKY_MSTR_CTRL_1, 0b00010100); //font overlay, double height text, 320x240 at 60 Hz;
	POKE(VKY_LAYER_CTRL_0, 0b00010000); //bitmap 0 in layer 0, bitmap 1 in layer 1
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
	bitmapSetActive(2);
	bitmapSetAddress(2,BITMAP_BASE);
	bitmapSetCLUT(0);
	
	bitmapSetVisible(0,false);
	bitmapSetVisible(1,false);
	bitmapSetVisible(2,true); //furthermost to act as a real background

/*
void mySetCar(uint8_t s, uint32_t addr, uint8_t size, uint8_t clut, uint8_t layer, uint8_t frame, uint16_t x, uint16_t y, uint8_t z, bool wantVisible, struct sprStatus *theCarPart) 
*/

	mySetCar(0, SPR_BACK_L, 32, 0, 1, 0, 128, 220, 0, true, &car_back_L);
	mySetCar(1, SPR_BACK_R, 32, 0, 1, 0, 160, 220, 0, true, &car_back_R);
	mySetCar(2, SPR_MIDDLE_L, 32, 0, 1, 0, 128, 220, 0, true, &car_mid_L);
	mySetCar(3, SPR_MIDDLE_R, 32, 0, 1, 0, 160, 220, 0, true, &car_mid_R);
	mySetCar(4, SPR_FRONT_L, 32, 0, 1, 0, 128, 220, 0, true, &car_front_L);
	mySetCar(5, SPR_FRONT_R, 32, 0, 1, 0, 160, 220, 0, true, &car_front_R);
	
	mySetCar(6, SPR_YEL_L, 32, 0, 1, 0, 165, ENMY_ENTRY_Y, 3, false, &car_yel_L);
	//sprite 7 is reserved for this yellow car
	car_yel_L.sy=1;
	
	
	setCarPos(128,220);

	//set NES_CTRL
	POKE(NES_CTRL,NES_CTRL_MODE_NES);

	POKE(MMU_IO_CTRL, 0x00);
	
}

void mySetCar(uint8_t s, uint32_t addr, uint8_t size, uint8_t clut, uint8_t layer, uint8_t frame, uint16_t x, uint16_t y, uint8_t z, bool wantVisible, struct sprStatus *theCarPart) 
{
	spriteDefine(s, addr, size, clut, layer);
	spriteSetPosition(s,x,y);
	spriteSetVisible(s,wantVisible);

	theCarPart->s = s;
	theCarPart->x = x;
	theCarPart->y = y;
	theCarPart->z = z;
	theCarPart->addr = addr;
	theCarPart->frame = frame;
	theCarPart->sx = 0;
	theCarPart->sy = 0;
}

			
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
	
	carForceTimer.units = TIMER_FRAMES;
	carForceTimer.absolute = getTimerAbsolute(TIMER_FRAMES) + TIMER_CAR_FORCE_DELAY;
	carForceTimer.cookie = TIMER_CAR_FORCE_COOKIE;
	setTimer(&carForceTimer);
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
	
	boostVSClock();
	initVS1053MIDI();
	
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
