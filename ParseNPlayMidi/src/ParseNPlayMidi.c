
//DEFINES
#define F256LIB_IMPLEMENTATION
#define SCREENCORNER 0xC000
#define MIDI_BASE 0x10000 //gives a nice 128kb until the parsed version happens

#define MIDI_PARSED 0x30000 //end of ram is 0x7FFFF
//offsets from MIDI_PARSED
#define AME_DELTA 0
#define AME_BYTECOUNT 4
#define AME_MSG 5


#define TIMER_FRAMES 0
#define TIMER_SECONDS 1
#define TIMER_MIDI_COOKIE 0

#define PREVIEW_LIMITER 30
#define MIDI_EVENT_FAR_SIZE 8




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






//INCLUDES
#include "f256lib.h"
#include <stdlib.h>
#include "../MacSource\midispec.c"
#include "../src/muUtils.h" //contains helper functions I often use
#include "../src/muMidi.h"  //contains basic MIDI functions I often use


//EMBEDS
EMBED(human2, "../assets/asayake.mid", 0x10000);

//STRUCTS
struct timer_t midiNoteTimer;


//GLOBALS
bigParsed theBigList;
bool gVerbo = false; //global verbose flag, gives more info throughout operations when set true
FILE *theMIDIfile;
uint16_t gFormat = 0; //holds the format type for the info provided after loading 0=single track, 1=multitrack
uint16_t trackcount = 0; // #number of tracks detected
uint32_t tick = 0; // #microsecond per tick
uint32_t gFileSize; //keeps track of how many bytes in the file
bool readyForNextNote = 1; //interrupt-led flag to signal the loop it's ready for the next MIDI event
uint16_t limiter = 400; //temporary limiter to the memory structure to do quick tests
uint16_t tempAddr; //used to compute addresses for pokes
uint16_t previewCount = 0; //used to preview count the tracks data
double fudge = 25.1658; //fudge factor when calculating time delays
uint16_t *parsers; //indices for the various type 1 tracks during playback

//FUNCTION PROTOTYPES
void sendAME(aMEPtr midiEvent); //sends a MIDI event message, either a 2-byte or 3-byte one
int16_t findPositionOfHeader(void);  //this opens a .mid file and ignores everything until 'MThd' is encountered	
void detectStructure(uint16_t startIndex); //checks the tempo, number of tracks, etc
uint8_t loadSMFile(char *name); //Opens the std MIDI file
int16_t getAndAnalyzeMIDI(void); //high level function that directs the reading and parsing of the MIDI file  
int8_t parse(uint16_t startIndex, bool wantCmds);
void wipeBigList(void);
uint32_t getTotalLeft(void);
void playmidi(void);
void adjustOffsets(void);
void resetTimer0(void);
void setTimer0(uint8_t, uint8_t, uint8_t);
uint32_t readTimer0(void);	
uint8_t isTimer0Done(void);

void goToPrison(uint16_t);

//the workhorse of MIDIPlay, "send A MIDI Event" sends a string of MIDI bytes into the midi FIFO

//sends a MIDI event message, either a 2-byte or 3-byte one
void sendAME(aMEPtr midiEvent)
	{
	POKE(MIDI_FIFO, midiEvent->msgToSend[0]);
	POKE(MIDI_FIFO, midiEvent->msgToSend[1]);
	if(midiEvent->bytecount == 3) POKE(MIDI_FIFO, midiEvent->msgToSend[2]);
	}
	

	
//checks the tempo, number of tracks, etc
void detectStructure(uint16_t startIndex)
	{	
    uint32_t size = 0; //size of midi byte data count
    uint32_t trackLength = 0; //size in bytes of the current track
    uint16_t tdiv=0; //time per division in ticks
    uint32_t i = startIndex; // #main array parsing index
    uint32_t j=0;
    uint16_t currentTrack=0; //index for the current track
	  
    if(gVerbo) 
    	{
    	printf("MIDI file header\n");
    	printf("-------------\n");    
    	}
    i+=4;
    
    if(gVerbo) printf("size: ");
    size =  (uint32_t)((uint8_t) FAR_PEEK(MIDI_BASE+i+3));
    size += (uint32_t)((uint8_t) FAR_PEEK(MIDI_BASE+i+2))<<8;
    size += (uint32_t)((uint8_t) FAR_PEEK(MIDI_BASE+i+1))<<16;
    size += (uint32_t)((uint8_t) FAR_PEEK(MIDI_BASE+i))<<24;
    if(gVerbo) printf("%lu\n",size);
	i+=4;

    if(gVerbo) printf("format type (0=single track, 1=multitrack): ");
    gFormat = (uint16_t)(
     	           (uint8_t) (FAR_PEEK(MIDI_BASE+i+1))
    			  +(uint8_t) (FAR_PEEK(MIDI_BASE+i)<<8)
    			  );
    if(gVerbo) printf("%d\n",gFormat);
    i+=2;
 
    if(gVerbo) printf("Track count: ");
    trackcount = (uint16_t)(
     	           (uint8_t)(FAR_PEEK(MIDI_BASE+i+1))  
    			  +(uint8_t)((FAR_PEEK(MIDI_BASE+i)<<8)) 
    			  );
    if(gVerbo) printf("%d\n",trackcount);
    i+=2;
    
    if(gVerbo) printf("Time in ticks per division: ");
    tdiv = (uint16_t)(
     	           (uint8_t)(FAR_PEEK(MIDI_BASE+i+1))  
    			  +(uint8_t)((FAR_PEEK(MIDI_BASE+i)<<8)) 
    			  );
    if(gVerbo) printf("%d\n",tdiv);
    i+=2;
    
	gFileSize = (uint32_t)human2_end - (uint32_t)human2_start;
    if(gVerbo) printf("total length in bytes= %lu\n",gFileSize);
    currentTrack=0;
   
   
        //start defining overal sizes in the big list, now that trackcount is known
     if((theBigList).hasBeenUsed == true)
       {
        //wipeBigList();
        }
     
	 theBigList.hasBeenUsed = true;
     theBigList.trackcount = trackcount;
     theBigList.TrackEventList = (aTOEPtr) malloc((sizeof(aTOE))*theBigList.trackcount);
	 
	parsers = (uint16_t *) malloc(sizeof(uint16_t) * trackcount);
	
    while(currentTrack < trackcount)
    	{
			parsers[currentTrack] = 0;
	    	currentTrack++;
	    	i+=4; //skips the MTrk string
	    	
	    	trackLength =  (uint32_t)((uint8_t) FAR_PEEK(MIDI_BASE+i+3));
	    	trackLength += (uint32_t)((uint8_t) FAR_PEEK(MIDI_BASE+i+2))<<8;
	    	trackLength += (uint32_t)((uint8_t) FAR_PEEK(MIDI_BASE+i+1))<<16;
	    	trackLength += (uint32_t)((uint8_t) FAR_PEEK(MIDI_BASE+i))<<24;
	        i+=4;
	        
	        i+=trackLength;
	       
    	} //end of parsing all tracks
		
	printf("\ndetectStructure\n");
	printf("hasBeenUsed %d trackcount %d TrackEvLst %08x\n",theBigList.hasBeenUsed, theBigList.trackcount, (uint16_t)theBigList.TrackEventList);

     for(j=0;j<theBigList.trackcount;j++) 
        {
			
        theBigList.TrackEventList[j].trackno = j;
        theBigList.TrackEventList[j].eventcount = 0;
        theBigList.TrackEventList[j].eventleft = 0;
		theBigList.TrackEventList[j].baseOffset = 0;
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
	
	for(i=0;i<400;i++) buffer[i] = FAR_PEEK(0x10000 + i);
	
    position = strstr(buffer, targetSequence);
	
	if(position != NULL)
		{
		thePosition = (int16_t)(position - buffer);
		return thePosition;
		}
	return -1;
	}	
	
	
//high level function that directs the reading and parsing of the MIDI file     
int16_t getAndAnalyzeMIDI(void)
	{
	int16_t indexToStart=0; //MThd should be at position 0, but it might not, so we'll find it
	
	//result = loadSMFile("human2.mid"); //actual process of asking for a midi file through std mac dialog
	
	indexToStart = findPositionOfHeader(); //find the start index of 'MThd'
	if(gVerbo) printf("Had to skip %d bytes to find MIDI Header tag\n",indexToStart);
	if(indexToStart == -1)
		{
		printf("ERROR: couldn't find a MIDI header in your file; it might be invalid\n");
		return -1;
		}
	detectStructure(indexToStart); //parse it a first time to get the format type and nb of tracks
	
	return indexToStart;
	}


//Opens the std MIDI file
uint8_t loadSMFile(char *name)
{
	uint32_t fileSize=0;
	char buffer[1024];
	size_t bytesRead;

	if(gVerbo) printf("Opening file: %s\n",name);
	theMIDIfile = fopen(name,"rb"); // open file in read mode
	if(theMIDIfile == NULL)
	{
		if(gVerbo) printf("Couldn't open the file: %s\n",name);
		return 1;
	}
	
	while ((bytesRead = fread(buffer, 1, sizeof(buffer), theMIDIfile)) > 0) 
	{ 
	fileSize += bytesRead;
	}
	
	if(gVerbo) printf("size of file: %lu\n",fileSize);
	
	return 0;
}

void lilpause(uint8_t timedelay)
{
	bool noteExitFlag = false;
	midiNoteTimer.absolute = getTimerAbsolute(TIMER_FRAMES) + timedelay;
	setTimer(&midiNoteTimer);
	noteExitFlag = false;
	while(!noteExitFlag)
	{
		kernelNextEvent();
		if(kernelEventData.type == kernelEvent(timer.EXPIRED))
		{
			switch(kernelEventData.timer.cookie)
			{
			case TIMER_MIDI_COOKIE:
				noteExitFlag = true;
				break;
			}
		}
	}
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


void playmiditype0(void)
{
	uint16_t currentIndex=0;
	uint16_t localTotalLeft=0;
	uint32_t whereTo; //in far memory, keeps adress of event to send
	uint32_t overFlow;
	aME msgGo;
	bool exitFlag = false;

	asm("sei");
	printf("\nCurrently playing type 0 standard midi file\n");
	
	localTotalLeft = getTotalLeft();
	printf("localTotalLeft %d\n", localTotalLeft);
	
	while(localTotalLeft > 0 && !exitFlag)
	{	
			currentIndex = parsers[0]; //get where we at with this delta
			whereTo  = (uint32_t)(theBigList.TrackEventList[0].baseOffset);
			whereTo += (uint32_t) ((uint32_t) currentIndex * (uint32_t) MIDI_EVENT_FAR_SIZE);
			
			msgGo.deltaToGo =  (((uint32_t)((FAR_PEEKW((uint32_t) MIDI_PARSED + (uint32_t) whereTo))) << 16)
							 |   (uint32_t) (FAR_PEEKW((uint32_t) MIDI_PARSED + (uint32_t) whereTo + (uint32_t) 2)));
			msgGo.bytecount = 	FAR_PEEK((uint32_t) MIDI_PARSED + (uint32_t) whereTo + (uint32_t) AME_BYTECOUNT);
			msgGo.msgToSend[0] = FAR_PEEK((uint32_t) MIDI_PARSED + (uint32_t) whereTo + (uint32_t) AME_MSG);
			msgGo.msgToSend[1] = FAR_PEEK((uint32_t) MIDI_PARSED + (uint32_t) whereTo + (uint32_t) AME_MSG+1);
			msgGo.msgToSend[2] = FAR_PEEK((uint32_t) MIDI_PARSED + (uint32_t) whereTo + (uint32_t) AME_MSG+2);

			if(msgGo.deltaToGo >0)
				{
					overFlow = msgGo.deltaToGo;
					while(overFlow > 0x00FFFFFF) //0x00FFFFFF is the max value of the timer0 we can do
					{
						printf("overflow");
						setTimer0(0xFF,0xFF,0xFF);
						while(isTimer0Done()==0);
						POKE(T0_PEND,0x10); //clear timer0 at 0x10
						overFlow = overFlow - 0x00FFFFFF; //reduce the max value one by one until there is a remainder smaller than the max amount
					}
					setTimer0((uint8_t)(overFlow&0x000000FF),
						  (uint8_t)((overFlow&0x0000FF00)>>8),
						  (uint8_t)((overFlow&0x00FF0000)>>16));
					while(isTimer0Done()==0);
					POKE(T0_PEND,0x10); //clear timer0 at 0x10				
				}
			sendAME(&msgGo);
			
			parsers[0]++;
			localTotalLeft--;
	}
	
	asm("cli");
}

void goToPrison(uint16_t prisonTime)
{
	midiNoteTimer.absolute = getTimerAbsolute(TIMER_FRAMES) + prisonTime;
	setTimer(&midiNoteTimer);
	while(true)
	{		
		kernelNextEvent();
		if(kernelEventData.type == kernelEvent(timer.EXPIRED))
			{
			switch(kernelEventData.timer.cookie)
				{
				//all user interface related to text update through a 1 frame timer is managed here
				case TIMER_MIDI_COOKIE:
					return;
					break;
				}
			}
		}
	}
	
void playmidi(void)
{
	uint16_t i;
	uint16_t lowestTrack=0, currentIndex=0;
	uint16_t localTotalLeft=0;
	uint32_t lowestTimeFound = 50000000;
	uint32_t whereTo; //in far memory, keeps adress of event to send
	uint16_t trackcount;
	uint32_t delta; //used to compare times
	uint32_t overFlow;
	bool exitFlag = false;
	aME msgGo;


	asm("sei");
	trackcount = theBigList.trackcount;

	printf("\nCurrently playing type 1 standard midi file\n");
	
	localTotalLeft = getTotalLeft();
	printf("localTotalLeft %d", localTotalLeft);
	
	while(localTotalLeft > 0 && !exitFlag)
		{
			
		//For loop attempt to find the most pressing event with the lowest time delta to go
		for(i=0; i<trackcount; i++)
			{
			if((theBigList.TrackEventList[lowestTrack].eventcount - 1) == parsers[lowestTrack]) continue; //this track is exhausted, go to next
			
			currentIndex = parsers[i]; //get where we at with this delta
			
			whereTo  = (uint32_t)(theBigList.TrackEventList[i].baseOffset);
			whereTo += (uint32_t)((uint32_t) currentIndex *(uint32_t)  MIDI_EVENT_FAR_SIZE);
			
			//lowestAddr = (uint32_t)(theBigList).TrackEventList[i].baseOffset + (uint32_t)(currentIndex * MIDI_EVENT_FAR_SIZE);
			
			delta =    (((uint32_t)((FAR_PEEKW((uint32_t) MIDI_PARSED + (uint32_t) whereTo))) << 16)
					   | (uint32_t) (FAR_PEEKW((uint32_t) MIDI_PARSED + (uint32_t) whereTo + (uint32_t) 2)));
					
			if(delta < lowestTimeFound) 
				{
				lowestTimeFound = delta;
				lowestTrack = i; //new record in this track
				}
			if(lowestTimeFound == 0) break; //not gonna find a more imminent event
			}  //end of the for loop for most imminent event

		//Do the event
			msgGo.deltaToGo = lowestTimeFound;
			msgGo.bytecount =    FAR_PEEK((uint32_t) MIDI_PARSED + (uint32_t) whereTo + (uint32_t) AME_BYTECOUNT);
			msgGo.msgToSend[0] = FAR_PEEK((uint32_t) MIDI_PARSED + (uint32_t) whereTo + (uint32_t) AME_MSG);
			msgGo.msgToSend[1] = FAR_PEEK((uint32_t) MIDI_PARSED + (uint32_t) whereTo + (uint32_t) AME_MSG+(uint32_t) 1);
			msgGo.msgToSend[2] = FAR_PEEK((uint32_t) MIDI_PARSED + (uint32_t) whereTo + (uint32_t) AME_MSG+(uint32_t) 2);
			
		if(lowestTimeFound==0) //do these 0 delay events right away, no need to involve a Time Manager
			{
			sendAME(&msgGo);
			}
		else 
			{ //for all the rest which have a time delay
				overFlow = msgGo.deltaToGo;
				while(overFlow > 0x00FFFFFF) //0x00FFFFFF is the max value of the timer0 we can do
				{
					printf("overflow");
					setTimer0(0xFF,0xFF,0xFF);
					while(isTimer0Done()==0);
					POKE(T0_PEND,0x10); //clear timer0 at 0x10
					overFlow = overFlow - 0x00FFFFFF; //reduce the max value one by one until there is a remainder smaller than the max amount
				}
				setTimer0((uint8_t)(overFlow&0x000000FF),
					  (uint8_t)((overFlow&0x0000FF00)>>8),
				      (uint8_t)((overFlow&0x00FF0000)>>16));
				while(isTimer0Done()==0)
					;				
                POKE(T0_PEND,0x10); //clear timer0 at 0x10
			    sendAME(&msgGo);

			}					
		//Advance the marker for the track that just did something, if there's more left
		
		if((theBigList.TrackEventList[lowestTrack].eventcount - 1) > parsers[lowestTrack]) parsers[lowestTrack]++;

		//lower every other pending events with the time delta we just did, this is destructive 
		if(lowestTimeFound >0)
			{
			for(i=0;i<trackcount;i++)
				{
					
				if(i==lowestTrack) continue; //lower all others, but not the one we just did

				whereTo  = (uint32_t)(theBigList.TrackEventList[i].baseOffset);
				whereTo += (uint32_t) ((uint32_t) parsers[i] * (uint32_t) MIDI_EVENT_FAR_SIZE);
			
				//lowestAddr = (theBigList).TrackEventList[i].baseOffset + currentIndex * MIDI_EVENT_FAR_SIZE;
				delta =   ((uint32_t)((FAR_PEEKW((uint32_t) MIDI_PARSED + (uint32_t) whereTo)))<<16 
						 | (uint32_t)(FAR_PEEKW((uint32_t) MIDI_PARSED + (uint32_t) whereTo + (uint32_t) 2))
						  );
					
				delta -= lowestTimeFound;

				FAR_POKEW((uint32_t) MIDI_PARSED + (uint32_t) whereTo, (uint16_t)((delta&0xFFFF0000)>>16));
				FAR_POKEW((uint32_t) MIDI_PARSED + (uint32_t) whereTo + (uint32_t) 2, (uint16_t)(delta&0x0000FFFF));
				}
			}
		lowestTimeFound = 50000000; //make it easy to find lower for next pass
		localTotalLeft--;
		}
	asm("cli");
	}

//gets a count of the total MIDI events that are relevant and left to play	
uint32_t getTotalLeft(void)
	{
	uint32_t sum=0;
	uint16_t i=0;
	uint16_t tc;

	tc 	= (theBigList).trackcount;
	
	for(i=0; i<tc; i++)
		{
		sum+=(theBigList).TrackEventList[i].eventleft;
		}
	return sum;
	}
	

void wipeBigList(void)
	{
	int i = 0, nbTracks = 0;
	nbTracks = (theBigList).trackcount;
	for(i=0; i< nbTracks; i++) //for every track
		{
		(theBigList).TrackEventList[i].eventcount=0;
		(theBigList).TrackEventList[i].eventleft=0;
		}
	(theBigList).trackcount=0;
	}
	
	
//assuming a byte buffer that starts with MThd, read all tracks and produce a structure of aMIDIEvent arrays
// wantCmds is when it's ready to record the relevant commands in an array
int8_t parse(uint16_t startIndex, bool wantCmds)
	{
    uint32_t size = 0; //size of midi byte data count
    uint32_t trackLength = 0; //size in bytes of the current track
    uint16_t tickPerBeat=0; //time per division in ticks
    uint32_t i = startIndex; // #main array parsing index
    uint32_t j=0;
    uint16_t currentTrack=0; //index for the current track
    uint8_t last_cmd = 0x00;
    uint32_t currentI;
    uint32_t nValue, nValue2, nValue3, nValue4, timeDelta;
    uint8_t status_byte = 0x00, extra_byte = 0x00, extra_byte2 = 0x00;
    uint8_t meta_byte = 0x00;
    uint32_t data_byte = 0x00, data_byte2= 0x00, data_byte3 = 0x00, data_byte4= 0x00;
	uint32_t usPerBeat; //this is read off of a meta event 0xFF 0x51, 3 bytes long
   
	uint32_t whereTo=0; //where to write individual midi events in far memory
	
    //first pass will find the number of events to keep in the TOE (table of elements)
    //and initialize the myParsedEventList, an array of TOE arrays

    //second pass will actually record the midi events into the appropriate TOE for each track

    i+=4; //skips MThd midi file header 4 character id string
    
    size =  (uint32_t)((uint8_t) FAR_PEEK(MIDI_BASE+i+3));
    size += (uint32_t)((uint8_t) FAR_PEEK(MIDI_BASE+i+2))<<8;
    size += (uint32_t)((uint8_t) FAR_PEEK(MIDI_BASE+i+1))<<16;
    size += (uint32_t)((uint8_t) FAR_PEEK(MIDI_BASE+i))<<24;
	i+=4;

printf(" size=%08lx",size);  
 
    gFormat = (uint16_t)(
     	           (uint8_t) (FAR_PEEK(MIDI_BASE+i+1))
    			  +(uint8_t) (FAR_PEEK(MIDI_BASE+i)<<8)
    			  );
    i+=2;

printf(" format=%d",gFormat);  
 
    trackcount = (uint16_t)(
     	           (uint8_t)(FAR_PEEK(MIDI_BASE+i+1))  
    			  +(uint8_t)((FAR_PEEK(MIDI_BASE+i)<<8)) 
    			  );
    i+=2;

printf(" trackcount=%d",trackcount);  
    
    tickPerBeat = (uint16_t)(
     	           (uint8_t)(FAR_PEEK(MIDI_BASE+i+1))  
    			  +(uint8_t)((FAR_PEEK(MIDI_BASE+i)<<8)) 
    			  );
    i+=2;
printf(" tickPerBeat=%d",tickPerBeat);   
 
	gFileSize = (uint32_t)human2_end - (uint32_t)human2_start;
    currentTrack=0;
    
printf(" filesize=%08lx\n",gFileSize);	
		
    while(currentTrack < trackcount)
    	{
		previewCount=0; //start fresh for the prewiew of the midi messages per track, to be shown in columns of printf output 
		currentTrack++;
		
//track column headers
//textSetColor(1,0); printf("Tr. %02d offset at %08x\n",currentTrack, theBigList.TrackEventList[currentTrack-1].baseOffset);

		i+=4; //skip MTrk header 4 character string
		
		trackLength =  (uint32_t)((uint8_t) FAR_PEEK(MIDI_BASE+i+3));
		trackLength += (uint32_t)((uint8_t) FAR_PEEK(MIDI_BASE+i+2))<<8;
		trackLength += (uint32_t)((uint8_t) FAR_PEEK(MIDI_BASE+i+1))<<16;
		trackLength += (uint32_t)((uint8_t) FAR_PEEK(MIDI_BASE+i))<<24;

        i+=4; //skip track length in bytes
       
    	
    	last_cmd = 0x00;
    	currentI = i;
		
    	while(i < (trackLength + currentI))
    		{

    		nValue = 0x00000000;
    		nValue2 = 0x00000000;
    		nValue3 = 0x00000000;
    		nValue4 = 0x00000000;
    		data_byte = 0x00000000;
    		
    		
    		status_byte = 0x00;
			
			nValue += (uint8_t)FAR_PEEK(MIDI_BASE+i);
			i++;
			if(nValue & 0x00000080)
				{
				nValue &= 0x0000007F;
				nValue <<= 7;
				nValue2 += (uint8_t)FAR_PEEK(MIDI_BASE+i);
				i++;
				if(nValue2 & 0x00000080)
					{
					nValue2 &= 0x0000007F;
					nValue2 <<= 7;
					nValue <<= 7;
					nValue3 += (uint8_t)FAR_PEEK(MIDI_BASE+i);
					i++;
					if(nValue3 & 0x00000080)
						{
						nValue3 &= 0x0000007F;
						nValue3 <<= 7;
						nValue2 <<= 7;
						nValue <<= 7;
						nValue4 += (uint8_t)FAR_PEEK(MIDI_BASE+i);
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
			
			
			//first, check for run-on commands that don't repeat the status_byte
			if(status_byte < 0x80)
				{					
			printf("---- %02x %02x %02x ----", status_byte, extra_byte, extra_byte2);
			hitspace();
				extra_byte = status_byte; //recycle the byte to its proper destination
				status_byte = last_cmd;
				extra_byte2 = FAR_PEEK(MIDI_BASE+i+1); //redo: be ready for 3 bytes midi events
				i--; //go back 1 spot so it can read the data properly			
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
									 
					printf(" us per beat %08lx ",usPerBeat);
					
					//if you divide usPerBeat by tickPerBeat, 
					//you get the duration in microseconds per tick, ready to be multiplied	by the events' deltaTimes to get delays in us

					tick = (uint32_t)usPerBeat/((uint32_t)tickPerBeat);
					tick = (uint32_t)((double)(fudge * (double)tick));
					
										
					}
				else if(meta_byte == MetaSMPTEOffset)
					{
					i+=6;
					}
				else if(meta_byte == MetaTimeSignature)
					{
					i+=5;
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
					theBigList.TrackEventList[currentTrack-1].eventcount++; 

					}
				if(wantCmds) //prep the MIDI event
					{
						
					whereTo  = (uint32_t)(theBigList.TrackEventList[currentTrack-1].baseOffset);
					whereTo += (uint32_t)( (uint32_t)theBigList.TrackEventList[currentTrack-1].eventleft * (uint32_t) MIDI_EVENT_FAR_SIZE);
					
					
					FAR_POKEW((uint32_t) MIDI_PARSED + (uint32_t) whereTo    			, (uint16_t) (( (uint32_t)(   (uint32_t)tick * (uint32_t)timeDelta) & 0xFFFF0000)  >>16))  ;
					FAR_POKEW((uint32_t) MIDI_PARSED + (uint32_t) whereTo + (uint32_t) 2, (uint16_t) (( (uint32_t)(   (uint32_t)tick * (uint32_t)timeDelta) & 0x0000FFFF)      ))  ;
				
					FAR_POKE((uint32_t) MIDI_PARSED + (uint32_t) whereTo + (uint32_t) AME_BYTECOUNT, 2);
					FAR_POKE((uint32_t) MIDI_PARSED + (uint32_t) whereTo + (uint32_t) AME_MSG, status_byte);
					FAR_POKE((uint32_t) MIDI_PARSED + (uint32_t) whereTo + (uint32_t) AME_MSG+(uint32_t) 1, extra_byte);
					FAR_POKE((uint32_t) MIDI_PARSED + (uint32_t) whereTo + (uint32_t) AME_MSG+(uint32_t) 2, 0x00); //not needed but meh
					
					theBigList.TrackEventList[currentTrack-1].eventleft++;

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
					theBigList.TrackEventList[currentTrack-1].eventcount++;
					}
				if(wantCmds) //prep the MIDI event
					{
						
					whereTo =  (uint32_t)(theBigList.TrackEventList[currentTrack-1].baseOffset);
					whereTo += (uint32_t)( (uint32_t) theBigList.TrackEventList[currentTrack-1].eventleft * (uint32_t) MIDI_EVENT_FAR_SIZE);
					
					
					FAR_POKEW((uint32_t) MIDI_PARSED + (uint32_t) whereTo    			, (uint16_t) (( (uint32_t)(   (uint32_t)tick * (uint32_t)timeDelta) & 0xFFFF0000)  >>16))  ;
					FAR_POKEW((uint32_t) MIDI_PARSED + (uint32_t) whereTo + (uint32_t) 2, (uint16_t) (( (uint32_t)(   (uint32_t)tick * (uint32_t)timeDelta) & 0x0000FFFF)      ))  ;
				
					FAR_POKE((uint32_t) MIDI_PARSED + (uint32_t) whereTo + (uint32_t) AME_BYTECOUNT, 3);
					FAR_POKE((uint32_t) MIDI_PARSED + (uint32_t) whereTo + (uint32_t) AME_MSG, status_byte);
					FAR_POKE((uint32_t) MIDI_PARSED + (uint32_t) whereTo + (uint32_t) AME_MSG+(uint32_t) 1, extra_byte);
					FAR_POKE((uint32_t) MIDI_PARSED + (uint32_t) whereTo + (uint32_t) AME_MSG+(uint32_t) 2, extra_byte2);
					
					theBigList.TrackEventList[currentTrack-1].eventleft++;				

					}
				i+=2; //advance the index either way
				}// end of 3-data-byter events
				

    		else
    			{
    			//printf("\n ---Unrecognized event sb= %02x",status_byte);
    			}
    		last_cmd = status_byte;
    		
    		} //end of parsing a track
    	} //end of parsing all tracks
		
if(wantCmds)
{	
textSetColor(6,0);
 for(j=0;j<theBigList.trackcount;j++) 
	{	
	printf("track %02d eventcount %05d eventleft %05d addr %08lx\n",
			theBigList.TrackEventList[j].trackno + 1, 
			theBigList.TrackEventList[j].eventcount, 
			theBigList.TrackEventList[j].eventleft,
			theBigList.TrackEventList[j].baseOffset);
	}
}
		return 0;
     }	 //end of parse function
     
void adjustOffsets()	 
{		
	uint16_t i=0,k=0, currentEventCount=0;	
	
	for(i=0;i<theBigList.trackcount;i++)
	{
		theBigList.TrackEventList[i].baseOffset = (uint32_t)0;
		printf("curT=%d start off: %08lx\n",i,theBigList.TrackEventList[i].baseOffset);
		for(k=0;k<i;k++) //do this for all tracks before it
			{
			currentEventCount=theBigList.TrackEventList[k].eventcount;
			printf("t=%d count=%05d add=%08lx\n",k,currentEventCount,(uint32_t)(currentEventCount*MIDI_EVENT_FAR_SIZE));
			theBigList.TrackEventList[i].baseOffset += (uint32_t)(currentEventCount*MIDI_EVENT_FAR_SIZE ); //skip to a previous track
			}	
		printf("final off: %08lx\n",theBigList.TrackEventList[i].baseOffset);
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
int main(int argc, char *argv[]) {
	bool isDone = false;
	int16_t indexStart = 0;
	
	
	POKE(MMU_IO_CTRL, 0x00);
	POKE(VKY_MSTR_CTRL_0, 0x4F); //graphics and tile enabled
	POKE(VKY_MSTR_CTRL_1, 0x00); //320x240 at 60 Hz;
	textSetDouble(false, false);

	POKE(MIDI_FIFO, 0xC0);
	POKE(MIDI_FIFO, 0);
resetInstruments(false);
	
	//Prep the big digested data structure for when the MIDI file is analyzed and ready to play
	theBigList.hasBeenUsed = false;
	theBigList.trackcount = 0;
	theBigList.TrackEventList = (aTOEPtr)NULL;
	
	//Prep timers		
	midiNoteTimer.units = TIMER_FRAMES;
	midiNoteTimer.cookie = TIMER_MIDI_COOKIE;

	indexStart = getAndAnalyzeMIDI();
	
	setTimer0(0,0,0);
	
	if(indexStart!= -1)
	{
		parse(indexStart,false); //count the events and prep the mem allocation for the big list
		adjustOffsets();
		parse(indexStart,true); //load up the actual event data in the big list
		if(gFormat == 0) playmiditype0();
		else playmidi();	
		printf("Playback ended.\n");
	}
	while(!isDone)
	{
		
	}
	
	return 0;}
