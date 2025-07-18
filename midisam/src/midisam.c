#define F256LIB_IMPLEMENTATION
#include "f256lib.h"
#include "../src/muUtils.h" //contains helper functions I often use
#include "../src/setup.h" //contains helper functions I often use
#include "../src/muVS1053b.h" //VS1053b stuff
#include "../src/muMidi.h"  //contains basic MIDI functions I often use
#include "../src/timer0.h"  //contains basic cpu based timer0 functions I often use

#define MIDI_BASE   0x20000 //gives a nice 07ffkb until the parsed version happens
#define MIDI_PARSED 0x50000 //end of ram is 0x7FFFF, gives a nice 256kb of parsed midi

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
uint8_t loadSMFile(char *, uint32_t);
int16_t getAndAnalyzeMIDI(struct midiRecord *, struct bigParsedEventList *);
void detectStructure(uint16_t, struct midiRecord *, struct bigParsedEventList *);
int16_t findPositionOfHeader(void);
void adjustOffsets(struct bigParsedEventList *);	
int8_t parse(uint16_t, bool, struct midiRecord *, struct bigParsedEventList *);
uint8_t playmidi(struct midiRecord *, struct bigParsedEventList *);
uint32_t getTotalLeft(struct bigParsedEventList *);
void sendAME(aMEPtr, bool);
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



//sends a MIDI event message, either a 2-byte or 3-byte one
void sendAME(aMEPtr midiEvent, bool wantAlt) {
		uint8_t chan = (midiEvent->msgToSend[0])&0x0F;
		uint8_t bufferLocation = (midiEvent->msgToSend[1])>>4;
		uint8_t bitLocation = (midiEvent->msgToSend[1])%8;
	POKE(wantAlt?MIDI_FIFO_ALT:MIDI_FIFO, midiEvent->msgToSend[0]);
	POKE(wantAlt?MIDI_FIFO_ALT:MIDI_FIFO, midiEvent->msgToSend[1]);
	
	if(midiEvent->bytecount == 3) {
		POKE(wantAlt?MIDI_FIFO_ALT:MIDI_FIFO, midiEvent->msgToSend[2]);
		
	}
	if((midiEvent->msgToSend[0] & 0xF0) == 0x90) 
		{
			shimmerChanged[chan][bufferLocation]=true; //mark this channel as changed
			SET_BIT(shimmerBuffer[chan][bufferLocation],bitLocation);
			POKE(disp[chan],midiEvent->msgToSend[1]<<2);
		}
			
	else if((midiEvent->msgToSend[0] & 0xF0) == 0x80)
		{
			shimmerChanged[chan][bufferLocation]=true; //mark this channel as changed
			CLEAR_BIT(shimmerBuffer[chan][bufferLocation],bitLocation);
			POKE(disp[chan],0);
		}
	else if((midiEvent->msgToSend[0] & 0xF0) == 0xC0)
	{
		 updateInstrumentDisplay(midiEvent->msgToSend[0] & 0x0F, midiEvent->msgToSend[1]); 
		 //send instrument change to the other MIDI device as well! Note the reverse lambda writing
		 POKE(wantAlt?MIDI_FIFO:MIDI_FIFO_ALT, midiEvent->msgToSend[0]);
		 POKE(wantAlt?MIDI_FIFO:MIDI_FIFO_ALT, midiEvent->msgToSend[1]);
	}
	//for EMWhite, double up the MIDI message towards the UART port
	if(CLONE_TO_UART_EMWHITE) {
	POKE(1,0);
	POKE(0xD630, midiEvent->msgToSend[0]);
	POKE(0xD630, midiEvent->msgToSend[1]);
	if(midiEvent->bytecount == 3) POKE(0xD630, midiEvent->msgToSend[2]);
	}
	}
	
//Opens the std MIDI file


uint8_t loadSMFile(char *name, uint32_t targetAddress) {
	FILE *theMIDIfile;
	uint8_t buffer[255];
	size_t bytesRead = 0;
	uint32_t totalBytesRead = 0;
	uint16_t i=0;

	theMIDIfile = fopen(name,"rb"); // open file in read mode
	if(theMIDIfile == NULL)
	{
		return 1;
	}

	while ((bytesRead = fread(buffer, sizeof(uint8_t), 250, theMIDIfile))>0)
			{ 
			buffer[0]=buffer[0];	
			//dump the buffer into a special RAM area
			for(i=0;i<bytesRead;i++)
				{
				FAR_POKE((uint32_t)targetAddress+(uint32_t)totalBytesRead+(uint32_t)i,buffer[i]);
				}
			totalBytesRead += (uint32_t) bytesRead;
			if(bytesRead < 250) break;
			}
	fclose(theMIDIfile);
	return 0;
}

//high level function that directs the reading and parsing of the MIDI file     
int16_t getAndAnalyzeMIDI(struct midiRecord *rec, struct bigParsedEventList *list) {		
	int16_t indexToStart=0; //MThd should be at position 0, but it might not, so we'll find it
	indexToStart = findPositionOfHeader(); //find the start index of 'MThd'	
	if(indexToStart == -1)	
		{
		return -1;
		}
	detectStructure(indexToStart, rec, list); //parse it a first time to get the format type and nb of tracks
	return indexToStart;
	}

//checks the tempo, number of tracks, etc
void detectStructure(uint16_t startIndex, struct midiRecord *rec, struct bigParsedEventList *list)
	{	
    uint32_t trackLength = 0; //size in bytes of the current track
    uint32_t i = startIndex; // #main array parsing index
    uint32_t j=0;
    uint16_t currentTrack=0; //index for the current track
	  
    i+=4; //skip header tag  
	i+=4; //skip SIZE which is always 6 anyway

    rec->format = 
     	           (uint16_t) (FAR_PEEK(MIDI_BASE+i+1))
    			  |(uint16_t) (FAR_PEEK(MIDI_BASE+i)<<8)
    			  ;
    i+=2;
 
    rec->trackcount = 
     	           (uint16_t)(FAR_PEEK(MIDI_BASE+i+1))  
    			  |(uint16_t)((FAR_PEEK(MIDI_BASE+i)<<8)) 
    			  ;
    i+=2;
    
    rec->tick = 
     	           (uint16_t)(FAR_PEEK(MIDI_BASE+i+1))  
    			  |(uint16_t)((FAR_PEEK(MIDI_BASE+i)<<8)) 
    			  ;
    i+=2;

	currentTrack=0;
     
	list->hasBeenUsed = true;
	list->trackcount = rec->trackcount;
	list->TrackEventList = (aTOEPtr) malloc((sizeof(aTOE)) * list->trackcount);
	 
	rec->parsers = (uint16_t *) malloc(sizeof(uint16_t) * rec->trackcount);
	
    while(currentTrack < rec->trackcount)
    	{
			rec->parsers[currentTrack] = 0;
	    	currentTrack++;
	    	i+=4; //skips the MTrk string
	    	
	    	trackLength =  (((uint32_t)(FAR_PEEK(MIDI_BASE+i)))<<24) 
		             | (((uint32_t)(FAR_PEEK(MIDI_BASE+i+1)))<<16)
					 | (((uint32_t)(FAR_PEEK(MIDI_BASE+i+2)))<<8)
					 |  ((uint32_t)(FAR_PEEK(MIDI_BASE+i+3)));
	        i+=4;
	        
	        i+=trackLength;
	       
    	} //end of parsing all tracks

     for(j=0;j<list->trackcount;j++) 
        {		
        list->TrackEventList[j].trackno = j;
        list->TrackEventList[j].eventcount = 0;
		list->TrackEventList[j].baseOffset = 0;
		}
	}


//this opens a .mid file and ignores everything until 'MThd' is encountered	
int findPositionOfHeader() {
	char targetSequence[] = "MThd";
    char *position;
    int thePosition = 0;
	char buffer[64];
	int i=0;
	
	for(i=0;i<64;i++) buffer[i] = FAR_PEEK(MIDI_BASE + i);
	
    position = strstr(buffer, targetSequence);

	if(position != NULL)
		{
		thePosition = (int)(position - buffer);	
		return thePosition;
		}
	return -1;
	}	

void adjustOffsets(struct bigParsedEventList *list) {		
	uint16_t i=0,k=0, currentEventCount=0;	
	
	for(i=0;i<list->trackcount;i++)
	{
		list->TrackEventList[i].baseOffset = (uint32_t)0;
		for(k=0;k<i;k++) //do this for all tracks before it
			{
			currentEventCount=list->TrackEventList[k].eventcount;
			list->TrackEventList[i].baseOffset += (uint32_t)(currentEventCount*MIDI_EVENT_FAR_SIZE ); //skip to a previous track
			}
	}	
}	

//assuming a byte buffer that starts with MThd, read all tracks and produce a structure of aMIDIEvent arrays
// wantCmds is when it's ready to record the relevant commands in an array
int8_t parse(uint16_t startIndex, bool wantCmds, struct midiRecord *rec, struct bigParsedEventList *list) {
    uint32_t trackLength = 0; //size in bytes of the current track
    uint32_t i = startIndex; // #main array parsing index
    uint16_t currentTrack=0; //index for the current track
	uint32_t tempCalc=0;
    uint8_t last_cmd = 0x00;
    uint32_t currentI;
	uint32_t timer0PerTick=0; //used to compute the time delta for timer0 for a tick unit of midi event
    uint32_t usPerTick=0; 	//microsecond per tick quantity
	uint16_t interestingIndex=0;
    uint32_t nValue, nValue2, nValue3, nValue4, timeDelta;
    uint8_t status_byte = 0x00, extra_byte = 0x00, extra_byte2 = 0x00;
    uint8_t meta_byte = 0x00;
    uint32_t data_byte = 0x00, data_byte2= 0x00, data_byte3 = 0x00, data_byte4= 0x00;
	uint32_t usPerBeat=500000; //this is read off of a meta event 0xFF 0x51, 3 bytes long
    bool lastCmdPreserver = false;
	uint32_t superTotal=0; //in uSeconds, the whole song
	
	uint32_t whereTo=0; //where to write individual midi events in far memory
	
    //first pass will find the number of events to keep in the TOE (table of elements)
    //and initialize the myParsedEventList, an array of TOE arrays

    //second pass will actually record the midi events into the appropriate TOE for each track

    i+=4; //skips 'MThd' midi file header 4 character id string 
	i+=4; //skip size since it's always 6

    rec->format = 
     	           (uint16_t) (FAR_PEEK(MIDI_BASE+i+1))
    			  |(uint16_t) (FAR_PEEK(MIDI_BASE+i)<<8)
    			  ;
    i+=2;

    rec->trackcount = 
     	           (uint16_t)(FAR_PEEK(MIDI_BASE+i+1))  
    			  |(uint16_t)((FAR_PEEK(MIDI_BASE+i)<<8)) 
    			  ;
    i+=2;

    rec->tick = (uint16_t)(
     	           (uint16_t)(FAR_PEEK(MIDI_BASE+i+1))  
    			  |(uint16_t)((FAR_PEEK(MIDI_BASE+i)<<8)) 
    			  );
    i+=2;
	
    currentTrack=0;
    
    while(currentTrack < rec->trackcount)
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
									 
					
					//if you divide usPerBeat by tick per beat, 
					//you get the duration in microseconds per tick, ready to be multiplied	by the events' deltaTimes to get delays in us

					usPerTick = (uint32_t)usPerBeat/((uint32_t)rec->tick);
					timer0PerTick = (uint32_t)((double)usPerTick * (double)rec->fudge); //convert to the units of timer0
					rec->bpm = (uint16_t) ((uint32_t)6E7/((uint32_t)usPerBeat));
					
			
					}
				else if(meta_byte == MetaSMPTEOffset)
					{
					i+=6;
					}
				else if(meta_byte == MetaTimeSignature)
					{
					i++; //skip, it should be a constant 0x04 here
					rec->nn = (uint8_t)FAR_PEEK(MIDI_BASE+i);
					i++;
					rec->dd = (uint8_t)FAR_PEEK(MIDI_BASE+i);
					i++;
					rec->cc = (uint8_t)FAR_PEEK(MIDI_BASE+i);
					i++;
					rec->bb = (uint8_t)FAR_PEEK(MIDI_BASE+i);
					i++;
					}
				else if(meta_byte == MetaKeySignature)
					{
					i+=3;
					}
				else if(meta_byte == MetaSequencerSpecific)
					{
					continue;
					}
			    
				} //end if for meta events
			//Third, deal with regular MIDI commands
			
			//MIDI commands with only 1 data byte
			//Program change   0xC_
			//Channel Pressure 0xD_
			else if(status_byte >= 0xC0 && status_byte <= 0xDF)
				{
				if(wantCmds == false) //merely counting here
					{
					list->TrackEventList[currentTrack].eventcount++; 
					}
				if(wantCmds) //prep the MIDI event
					{
						
					whereTo  = (uint32_t)(list->TrackEventList[currentTrack].baseOffset);
					whereTo += (uint32_t)( (uint32_t)interestingIndex * (uint32_t) MIDI_EVENT_FAR_SIZE);
					
					tempCalc = timer0PerTick * timeDelta;
					superTotal +=  (uint32_t)(tempCalc)>>3; //it has to fit, otherwise uint32_t limits are reached for a full song!
					
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
					list->TrackEventList[currentTrack].eventcount++;
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
					whereTo =  (uint32_t)(list->TrackEventList[currentTrack].baseOffset);
					whereTo += (uint32_t)( (uint32_t)interestingIndex * (uint32_t) MIDI_EVENT_FAR_SIZE);
					
					tempCalc = timer0PerTick * timeDelta;
					superTotal +=  (uint32_t)(tempCalc)>>3; //it has to fit, otherwise uint32_t limits are reached for a full song!
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

		rec->totalDuration = superTotal;
		return 0;
     }	 //end of parse function
     

//non-destructive version
uint8_t playmidi(struct midiRecord *rec, struct bigParsedEventList *list) {
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
	uint8_t progTrack=0;
	uint16_t progressSeconds=0;
	byte oldsec=0;
	
	aME msgGo;
	trackcount = list->trackcount;
	
	soundBeholders=(uint32_t*)malloc(trackcount * sizeof(uint32_t));
	
	localTotalLeft = getTotalLeft(list);
	
	for(i=0;i<trackcount;i++) //pick their first deltas to start things
	{
		if(list->TrackEventList[i].eventcount == 0) //empty track, avoid picking the delta of the 1st event of next track
		{
			soundBeholders[i]=0;
			continue;
		}
		whereTo  = (uint32_t)(list->TrackEventList[i].baseOffset);
		soundBeholders[i] =  (((uint32_t)((FAR_PEEKW((uint32_t) MIDI_PARSED + (uint32_t) whereTo))) << 16)
							|  (uint32_t) (FAR_PEEKW((uint32_t) MIDI_PARSED + (uint32_t) whereTo + (uint32_t) 2)));
	}

    shimmerTimer.cookie = TIMER_SHIMMER_COOKIE;
	shimmerTimer.absolute = getTimerAbsolute(TIMER_FRAMES)+TIMER_SHIMMER_DELAY;
	setTimer(&shimmerTimer);
	while(localTotalLeft > 0 && !exitFlag)
		{
		lowestTimeFound = 0xFFFFFFFF; //make it easy to find lower than this
	
		if(!isPaused)
			{	
				//For loop attempt to find the most pressing event with the lowest time delta to go
				for(i=0; i<trackcount; i++)
					{
					if(rec->parsers[i] >= (list->TrackEventList[i].eventcount)) continue; //this track is exhausted, go to next
					
					delta = soundBeholders[i];

					if(delta == 0)
					{
						lowestTimeFound = 0;
						lowestTrack = i;
						
						whereToLowest  = (uint32_t)(list->TrackEventList[i].baseOffset);
						whereToLowest += (uint32_t)((uint32_t) rec->parsers[i] *(uint32_t)  MIDI_EVENT_FAR_SIZE);
					
						break; //will not find better than 0 = immediately
					}
					//is it the lowest found yet?
					if(delta < lowestTimeFound) 
						{
						lowestTimeFound = delta;
						lowestTrack = i; //new record in this track, keep looking for better ones
						
						whereToLowest  = (uint32_t)(list->TrackEventList[i].baseOffset);
						whereToLowest += (uint32_t)((uint32_t) rec->parsers[i] *(uint32_t)  MIDI_EVENT_FAR_SIZE);
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
					sendAME(&msgGo, midiChip);
					if(optimizedMIDIShimmering()==1) return 1;
					}
				else 
					{ //for all the rest which have a time delay
						overFlow = lowestTimeFound;
						while(overFlow > 0x00FFFFFF) //0x00FFFFFF is the max value of the timer0 we can do
						{
							setTimer0(0xFF,0xFF,0xFF);
							while(isTimer0Done()==0) if(optimizedMIDIShimmering()==1) return 1;
								//delay up to maximum of 0x00FFFFFF = 2/3rds of a second
							POKE(T0_PEND,0x10); //clear timer0 at 0x10
							overFlow = overFlow - 0x00FFFFFF; //reduce the max value one by one until there is a remainder smaller than the max amount
						}
						//do the last delay that's under 2/3rds of a second
						setTimer0((uint8_t)(overFlow&0x000000FF),
							  (uint8_t)((overFlow&0x0000FF00)>>8),
							  (uint8_t)((overFlow&0x00FF0000)>>16));
						while(isTimer0Done()==0)  if(optimizedMIDIShimmering()==1) return 1;
											
						POKE(T0_PEND,0x10); //clear timer0 at 0x10
						sendAME(&msgGo, midiChip);
					}					
					
				//Advance the marker for the track that just did something
				rec->parsers[lowestTrack]+=1; 
				whereToLowest  = (uint32_t)(list->TrackEventList[lowestTrack].baseOffset);
				whereToLowest += (uint32_t)((uint32_t) rec->parsers[lowestTrack] *(uint32_t)  MIDI_EVENT_FAR_SIZE);
				//replenish the new delta here
				soundBeholders[lowestTrack] =    (((uint32_t)((FAR_PEEKW((uint32_t) MIDI_PARSED + (uint32_t) whereToLowest))) << 16)
												| (uint32_t) (FAR_PEEKW((uint32_t) MIDI_PARSED + (uint32_t) whereToLowest + (uint32_t) 2)));
				for(i=0;i<trackcount;i++)
				{
					if(rec->parsers[i] >= (list->TrackEventList[i].eventcount)) continue;//that track was already exhausted
					if(i==lowestTrack) continue; //don't mess with the track that just acted
					soundBeholders[i] -= lowestTimeFound;
				}
				localTotalLeft--;
				
				kernelArgs->common.buf = &time_data;
				kernelArgs->common.buflen = sizeof(struct time_t);
				kernelCall(Clock.GetTime);
				
				if(time_data.seconds != oldsec) //only update if different
					{
					oldsec=time_data.seconds;
					progressSeconds++;			
					if(50*progressSeconds/(rec->totalSec) > progTrack)
						{
							progTrack++;
							updateProgress(progTrack);
						}
					} 	
				
			}	//end if is paused
	else if(optimizedMIDIShimmering()==1) return 1;		
	}//end of the whole playback
	return 0;
}//end function
	
uint8_t playmiditype0(struct midiRecord *rec, struct bigParsedEventList *list) {
	uint16_t localTotalLeft=0;
	uint32_t whereTo; //in far memory, keeps adress of event to send
	uint32_t overFlow;
	aME msgGo;
	bool exitFlag = false;
	uint8_t progTrack=0;
	uint16_t progressSeconds=0;
	byte oldsec=0;
	
	localTotalLeft = getTotalLeft(list);
	
	playbackTimer.absolute = getTimerAbsolute(TIMER_SECONDS)+1;
	setTimer(&playbackTimer);
	
	shimmerTimer.absolute = getTimerAbsolute(TIMER_FRAMES);
	setTimer(&shimmerTimer);
	
	while(localTotalLeft > 0 && !exitFlag)
	{	
	if(!isPaused)
	{	
			whereTo  = (uint32_t)(list->TrackEventList[0].baseOffset);
			whereTo += (uint32_t) ((uint32_t)rec->parsers[0] * (uint32_t) MIDI_EVENT_FAR_SIZE);
			
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
			sendAME(&msgGo, midiChip);
			if(optimizedMIDIShimmering()==1) return 1;
			rec->parsers[0]++;
			localTotalLeft--;
			
			kernelArgs->common.buf = &time_data;
			kernelArgs->common.buflen = sizeof(struct time_t);
			kernelCall(Clock.GetTime);
			
			if(time_data.seconds != oldsec) //only update if different
				{
				oldsec=time_data.seconds;
				progressSeconds++;			
				if(50*progressSeconds/(rec->totalSec) > progTrack)
					{
						progTrack++;
						updateProgress(progTrack);
					}
				} 
	} //end if is not paused

	else if(optimizedMIDIShimmering()==1) return 1;
	}//end of the whole playback
	return 0;
}//end function

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
	textGotoXY(1,25);textPrint("midiplayer v1.2E by Mu0n, April 2025");
	
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
	bigParsed theBigList; //master structure to keep note of all tracks, nb of midi events, for playback
	midiRec myRecord; //keeps parsed info about the midi file, tempo, etc, for info display

	uint8_t exitCode = 0;
	bool isDone = false; //to know when to exit the main loop; done playing
	int16_t indexStart = 0; //keeps note of the byte where the MIDI string 'MThd' is, sometimes they're not at the very start of the file!
	uint8_t i=0,j=0;
	uint8_t machineCheck=0;
	openAllCODEC(); //if the VS1053b is used, this might be necessary for some board revisions	
	//initVS1053MIDI();  //if the VS1053b is used

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
	
	initMidiRecord(&myRecord);
	initBigList(&theBigList);
	
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
	textGotoXY(0,25);printf("->Currently Loading file %s...",myRecord.fileName);
	resetInstruments(false); //resets all channels to piano, for sam2695
	midiShutUp(false); //ends trailing previous notes if any, for sam2695
	
	playbackTimer.units = TIMER_SECONDS;
	playbackTimer.cookie = TIMER_PLAYBACK_COOKIE;
		
	shimmerTimer.units = TIMER_FRAMES;
	shimmerTimer.cookie = TIMER_SHIMMER_COOKIE;
	
	if(loadSMFile(myRecord.fileName, MIDI_BASE))
	{
		printf("\nCouldn't open %s\n",myRecord.fileName);
		printf("Press space to exit.");
		hitspace();
		return 0;
	}

	indexStart = getAndAnalyzeMIDI(&myRecord, &theBigList);

	displayInfo(&myRecord);
	
		//assume control of system LED
	POKE(0xD6A0,0xE3);
	for(uint8_t i=0;i<3;i++)
	{
		
		POKE(0xD6A7+i,0);
		POKE(0xD6AA+i,0);
		POKE(0xD6AD+i,0);
		POKE(0xD6B3+i,0);
	}
	
	setTimer0(0,0,0);
	if(indexStart!=-1) //found a place to start in the loaded file, proceed to play
		{
		parse(indexStart,false, &myRecord, &theBigList); //count the events and prep the mem allocation for the big list
		adjustOffsets(&theBigList);
		extraInfo(&myRecord,&theBigList);
		parse(indexStart,true, &myRecord, &theBigList); //load up the actual event data in the big list
		wipeStatus();
		superExtraInfo(&myRecord);		
		
		while(!isDone)
			{
			initProgress();
			if(myRecord.format == 0) exitCode = playmiditype0(&myRecord, &theBigList);
			else exitCode = playmidi(&myRecord, &theBigList);
			
			if(exitCode == 1) isDone = true; //really quit no matter what
			if(repeatFlag == false) isDone=true;
			//reset the parsers
			for(i=0;i < theBigList.trackcount;i++)
				{
				myRecord.parsers[i] = 0;
				}
			}
		}	
	midiShutAllChannels(true);
	midiShutAllChannels(false);
	return 0;
	}
	