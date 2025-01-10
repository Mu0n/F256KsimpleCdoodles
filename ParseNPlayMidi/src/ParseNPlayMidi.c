
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

//INCLUDES
#include "f256lib.h"
#include <stdlib.h>
#include "../MacSource\midispec.c"
#include "../src/muUtils.h" //contains helper functions I often use
#include "../src/muMidi.h"  //contains basic MIDI functions I often use


//EMBEDS
EMBED(human2, "../assets/Bumble.mid", 0x10000);

//STRUCTS
struct timer_t midiNoteTimer;


//GLOBALS
bigParsed theBigList;
bool gVerbo = false; //global verbose flag, gives more info throughout operations when set true
FILE *theMIDIfile;
uint16_t gFormat = 0; //holds the format type for the info provided after loading 0=single track, 1=multitrack
uint16_t trackcount = 0; // #number of tracks detected
uint32_t tick = 0; // #millisecond per tick
uint32_t gFileSize; //keeps track of how many bytes in the file
bool readyForNextNote = 1; //interrupt-led flag to signal the loop it's ready for the next MIDI event
uint16_t limiter = 400; //temporary limiter to the memory structure to do quick tests
uint16_t tempAddr; //used to compute addresses for pokes
uint16_t previewCount = 0; //used to preview count the tracks data

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
	 
	 
    while(currentTrack < trackcount)
    	{
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
	char buffer[100];
	uint8_t i=0;
	
	for(i=0;i<100;i++) buffer[i] = FAR_PEEK(0x10000 + i);
	
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
	aME msgGo;

	bool exitFlag = false, prison= false;

	printf("\nCurrently playing type 0 standard midi file\n");
	
	localTotalLeft = getTotalLeft();
	printf("localTotalLeft %d\n", localTotalLeft);
	
	while(localTotalLeft > 0 && !exitFlag)
	{	
//For loop attempt to find the most pressing event with the lowest time delta to go
			currentIndex = (theBigList).TrackEventList[0].eventcount-(theBigList).TrackEventList[0].eventleft; //get where we at with this delta
			whereTo  = (uint32_t)(theBigList.TrackEventList[0].baseOffset);
			whereTo += (uint32_t) ((uint32_t) currentIndex * (uint32_t) MIDI_EVENT_FAR_SIZE);
			
			msgGo.deltaToGo =  (((uint32_t)((FAR_PEEKW((uint32_t) MIDI_PARSED + (uint32_t) whereTo))) << 16)
					 | (uint32_t)(FAR_PEEKW((uint32_t) MIDI_PARSED + (uint32_t) whereTo + (uint32_t) 2)));
					 
			msgGo.bytecount = 	FAR_PEEK((uint32_t) MIDI_PARSED + (uint32_t) whereTo + (uint32_t) AME_BYTECOUNT);
			msgGo.msgToSend[0] = FAR_PEEK((uint32_t) MIDI_PARSED + (uint32_t) whereTo + (uint32_t) AME_MSG);
			msgGo.msgToSend[1] = FAR_PEEK((uint32_t) MIDI_PARSED + (uint32_t) whereTo + (uint32_t) AME_MSG+1);
			msgGo.msgToSend[2] = FAR_PEEK((uint32_t) MIDI_PARSED + (uint32_t) whereTo + (uint32_t) AME_MSG+2);

			sendAME(&msgGo);

			midiNoteTimer.absolute = getTimerAbsolute(TIMER_FRAMES) + 2;
			setTimer(&midiNoteTimer);
			
			prison=false;
			while(!prison)
			{
			kernelNextEvent();
			if(kernelEventData.type == kernelEvent(timer.EXPIRED))
				{
				switch(kernelEventData.timer.cookie)
					{
					//all user interface related to text update through a 1 frame timer is managed here
					case TIMER_MIDI_COOKIE:
						prison = true;
						break;
					}
				}
			}
				textGotoXY(0,25);
				printf("from data: %d %02x %02x %02x |  ",FAR_PEEK(MIDI_PARSED + whereTo + AME_BYTECOUNT),
												  FAR_PEEK(MIDI_PARSED + whereTo + AME_MSG), 
												  FAR_PEEK(MIDI_PARSED + whereTo + AME_MSG+1),
												  FAR_PEEK(MIDI_PARSED + whereTo + AME_MSG+2)); 
				printf("msg to send: %d %02x %02x %02x", msgGo.bytecount, msgGo.msgToSend[0], msgGo.msgToSend[1], msgGo.msgToSend[2]);
				printf(" addr=%08lx",MIDI_PARSED + whereTo);
			(theBigList).TrackEventList[0].eventleft--;
			localTotalLeft--;
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
	aME msgGo;

	bool exitFlag = false, prison= false;

	trackcount = theBigList.trackcount;

	printf("\nCurrently playing type 1 standard midi file\n");
	
	localTotalLeft = getTotalLeft();
	printf("localTotalLeft %d", localTotalLeft);
	
	while(localTotalLeft > 0 && !exitFlag)
		{
			
		//For loop attempt to find the most pressing event with the lowest time delta to go
		for(i=0; i<trackcount; i++)
			{
			if((theBigList).TrackEventList[i].eventleft == 0) continue; //this track is exhausted, go to next
			
			currentIndex = theBigList.TrackEventList[i].eventcount-theBigList.TrackEventList[i].eventleft; //get where we at with this delta
			
			whereTo  = (uint32_t)(theBigList.TrackEventList[i].baseOffset);
			whereTo += (uint32_t)((uint32_t) currentIndex *(uint32_t)  MIDI_EVENT_FAR_SIZE);
			
			//lowestAddr = (uint32_t)(theBigList).TrackEventList[i].baseOffset + (uint32_t)(currentIndex * MIDI_EVENT_FAR_SIZE);
			
			delta =    (((uint32_t)((FAR_PEEKW((uint32_t) MIDI_PARSED + (uint32_t) whereTo))) << 16)
					 | (uint32_t)(FAR_PEEKW((uint32_t) MIDI_PARSED + (uint32_t) whereTo + (uint32_t) 2)));
					
			if(delta < lowestTimeFound) 
				{
				lowestTimeFound = delta;
				lowestTrack = i; //new record in this track
				}
			if(lowestTimeFound == 0) break; //not gonna find a more imminent event
			}  //end of the for loop for most imminent event

		//Do the event
		
		if(lowestTimeFound==0) //do these 0 delay events right away, no need to involve a Time Manager
			{
			msgGo.deltaToGo = lowestTimeFound;
			msgGo.bytecount =    FAR_PEEK((uint32_t) MIDI_PARSED + (uint32_t) whereTo + (uint32_t) AME_BYTECOUNT);
			msgGo.msgToSend[0] = FAR_PEEK((uint32_t) MIDI_PARSED + (uint32_t) whereTo + (uint32_t) AME_MSG);
			msgGo.msgToSend[1] = FAR_PEEK((uint32_t) MIDI_PARSED + (uint32_t) whereTo + (uint32_t) AME_MSG+(uint32_t) 1);
			msgGo.msgToSend[2] = FAR_PEEK((uint32_t) MIDI_PARSED + (uint32_t) whereTo + (uint32_t) AME_MSG+(uint32_t) 2);
			sendAME(&msgGo);
			readyForNextNote = 1;
			/*
			if(prev2 < PREVIEW_LIMITER)
				{
				textGotoXY(10,21 + prev2);
				printf("%02x%02x", msgGo.msgToSend[0], msgGo.msgToSend[1]); if(msgGo.bytecount==3)printf("%02x", msgGo.msgToSend[3]);
					prev2++;
				}
			*/
			//printf("msg %02x %02x \n", msgGo.bytecount, msgGo.msgToSend[0], msgGo.msgToSend[1]);
			
			}
		else 
			{ //for all the rest which have a time delay
		
			midiNoteTimer.absolute = getTimerAbsolute(TIMER_FRAMES) + 12;
			setTimer(&midiNoteTimer);
			prison=false;
			while(!prison)
				{
				kernelNextEvent();
				if(kernelEventData.type == kernelEvent(timer.EXPIRED))
					{
					switch(kernelEventData.timer.cookie)
						{
						//all user interface related to text update through a 1 frame timer is managed here
						case TIMER_MIDI_COOKIE:
							prison = true;
							break;
						}
					}
				}
			readyForNextNote = 0;

			msgGo.deltaToGo = lowestTimeFound;
			msgGo.bytecount =    FAR_PEEK((uint32_t) MIDI_PARSED + (uint32_t) whereTo + (uint32_t) AME_BYTECOUNT);
			msgGo.msgToSend[0] = FAR_PEEK((uint32_t) MIDI_PARSED + (uint32_t) whereTo + (uint32_t) (uint32_t) AME_MSG);
			msgGo.msgToSend[1] = FAR_PEEK((uint32_t) MIDI_PARSED + (uint32_t) whereTo + (uint32_t) AME_MSG+(uint32_t) 1);
			msgGo.msgToSend[2] = FAR_PEEK((uint32_t) MIDI_PARSED + (uint32_t) whereTo + (uint32_t) AME_MSG+(uint32_t) 2);
			sendAME(&msgGo);
			textGotoXY(0,25);
			printf("from data: %d %02x %02x %02x |  ",FAR_PEEK((uint32_t) MIDI_PARSED + (uint32_t) whereTo + (uint32_t) AME_BYTECOUNT),
												  FAR_PEEK((uint32_t) MIDI_PARSED + (uint32_t) whereTo + (uint32_t) AME_MSG), 
												  FAR_PEEK((uint32_t) MIDI_PARSED + (uint32_t) whereTo + (uint32_t) AME_MSG+(uint32_t) 1),
												  FAR_PEEK((uint32_t) MIDI_PARSED + (uint32_t) whereTo + (uint32_t) AME_MSG+(uint32_t) 2)); 
			printf("msg to send: %d %02x %02x %02x\n", msgGo.bytecount, msgGo.msgToSend[0], msgGo.msgToSend[1], msgGo.msgToSend[2]);
			/*
			if(prev2 < PREVIEW_LIMITER)
				{
				textGotoXY(10,21 + prev2);
				printf("%02x%02x", msgGo.msgToSend[0], msgGo.msgToSend[1]); if(msgGo.bytecount==3)printf("%02x", msgGo.msgToSend[3]);
				prev2++;
				}
			*/
			}					
		//Advance the marker for the track that just did something, if there's more left
		if((theBigList).TrackEventList[lowestTrack].eventleft > 0)
			{
			(theBigList).TrackEventList[lowestTrack].eventleft--;
			}
		//lower every other pending events with the time delta we just did, this is destructive 
		if(lowestTimeFound >0)
			{
			for(i=0;i<trackcount;i++)
				{
					
				if(i==lowestTrack) continue; //lower all others, but not the one we just did
				if(delta == 0) continue;
			
				currentIndex = (theBigList).TrackEventList[i].eventcount-(theBigList).TrackEventList[i].eventleft; //get where we at with this delta
				
				whereTo  = (uint32_t)(theBigList.TrackEventList[i].baseOffset);
				whereTo += (uint32_t) ((uint32_t) currentIndex * (uint32_t) MIDI_EVENT_FAR_SIZE);
			
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
    uint16_t bpm = 0; // #beat per minute
    uint32_t size = 0; //size of midi byte data count
    uint32_t trackLength = 0; //size in bytes of the current track
    uint16_t tdiv=0; //time per division in ticks
    uint32_t i = startIndex; // #main array parsing index
    uint32_t j=0;
    uint16_t currentTrack=0; //index for the current track
    uint8_t last_cmd = 0x00;
    uint32_t currentI;
    uint32_t nValue, nValue2, nValue3, nValue4, timeDelta;
    uint8_t status_byte = 0x00, extra_byte = 0x00, extra_byte2 = 0x00;
    uint8_t meta_byte = 0x00;
    uint32_t data_byte = 0x00, data_byte2= 0x00, data_byte3 = 0x00, data_byte4= 0x00;
   
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
    
    tdiv = (uint16_t)(
     	           (uint8_t)(FAR_PEEK(MIDI_BASE+i+1))  
    			  +(uint8_t)((FAR_PEEK(MIDI_BASE+i)<<8)) 
    			  );
    i+=2;
printf(" tdiv=%d",tdiv);   
 
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
				status_byte = last_cmd;
				i--; //go back 1 spot so it can read the data properly
				extra_byte  = FAR_PEEK(MIDI_BASE+i); //redo: be ready for 2 bytes midi events
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
					
					//version in positive milliseconds
					//bpm = 6E7/((data_byte2 << 16) | (data_byte3 << 8) | (data_byte4));
					//tick = 60000.0f/(bpm* tdiv);
					
					//version in positive 1/60ths of a second
					//bpm = 6E7/((data_byte2 << 16) | (data_byte3 << 8) | (data_byte4));
					//tick = 3600.0f/(bpm* tdiv);
					
					//version in negative microseconds
					bpm = 6E7/(    ( ((uint32_t)data_byte2)<<16 ) | 
								   ( ((uint32_t)data_byte3)<<8  ) | 
								     ((uint32_t)data_byte4)         );
					tick = (uint32_t)6E7/(uint32_t)bpm;
					tick = (uint32_t)tick/(uint32_t)tdiv;
										
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
					
					
					FAR_POKEW((uint32_t) MIDI_PARSED + (uint32_t) whereTo    			, (uint16_t) (( (tick * timeDelta) & 0xFFFF0000)  >>16))  ;
					FAR_POKEW((uint32_t) MIDI_PARSED + (uint32_t) whereTo + (uint32_t) 2, (uint16_t) (( (tick * timeDelta) & 0x0000FFFF)      ))  ;
				
					FAR_POKE((uint32_t) MIDI_PARSED + (uint32_t) whereTo + (uint32_t) AME_BYTECOUNT, 2);
					FAR_POKE((uint32_t) MIDI_PARSED + (uint32_t) whereTo + (uint32_t) AME_MSG, status_byte);
					FAR_POKE((uint32_t) MIDI_PARSED + (uint32_t) whereTo + (uint32_t) AME_MSG+(uint32_t) 1, extra_byte);
					FAR_POKE((uint32_t) MIDI_PARSED + (uint32_t) whereTo + (uint32_t) AME_MSG+(uint32_t) 2, 0x00); //not needed but meh
					
					theBigList.TrackEventList[currentTrack-1].eventleft++;
					/*
					printf("%02x %02x %02x %02x %02x %02x \n", status_byte, extra_byte, extra_byte2,
					FAR_PEEK(MIDI_PARSED + whereTo + AME_MSG),
					FAR_PEEK(MIDI_PARSED + whereTo + AME_MSG+1),
					FAR_PEEK(MIDI_PARSED + whereTo + AME_MSG+2));
					hitspace();*/
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
					
					
					FAR_POKEW((uint32_t) MIDI_PARSED + (uint32_t) whereTo,    (uint16_t)((   ( tick * timeDelta)  &   0xFFFF0000)>>16));
					FAR_POKEW((uint32_t) MIDI_PARSED + (uint32_t) whereTo + (uint32_t) 2,(uint16_t)((   ( tick * timeDelta)  &   0x0000FFFF)));
				
					FAR_POKE((uint32_t) MIDI_PARSED + (uint32_t) whereTo + (uint32_t) AME_BYTECOUNT, 3);
					FAR_POKE((uint32_t) MIDI_PARSED + (uint32_t) whereTo + (uint32_t) AME_MSG, status_byte);
					FAR_POKE((uint32_t) MIDI_PARSED + (uint32_t) whereTo + (uint32_t) AME_MSG+(uint32_t) 1, extra_byte);
					FAR_POKE((uint32_t) MIDI_PARSED + (uint32_t) whereTo + (uint32_t) AME_MSG+(uint32_t) 2, extra_byte2);
					
					theBigList.TrackEventList[currentTrack-1].eventleft++;				
					/*
					printf("%02x %02x %02x %02x %02x %02x \n", status_byte, extra_byte, extra_byte2,
					FAR_PEEK(MIDI_PARSED + whereTo + AME_MSG),
					FAR_PEEK(MIDI_PARSED + whereTo + AME_MSG+1),
					FAR_PEEK(MIDI_PARSED + whereTo + AME_MSG+2));
*/
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
	
	if(indexStart!= -1)
	{
		
//check the contents of human2.mid
		parse(indexStart,false); //count the events and prep the mem allocation for the big list
		adjustOffsets();
		parse(indexStart,true); //load up the actual event data in the big list
		playmiditype0();
		//if(gFormat == 0) playmiditype0();();
		//else playmidi();	
		//printf("Playback ended.\n");
	}
	while(!isDone)
	{
		
	}
	
	return 0;}
