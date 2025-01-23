//DEFINES
#define F256LIB_IMPLEMENTATION
#define MIDI_BASE   0x10000 //gives a nice 07ffkb until the parsed version happens
#define MIDI_PARSED 0x40000 //end of ram is 0x7FFFF, gives a nice 256kb of parsed midi

#define TIMER_FRAMES 0
#define TIMER_SECONDS 1
#define TIMER_PLAYBACK_COOKIE 0
#define TIMER_DELAY_COOKIE 1

#include "../src/muUtils.h" //contains helper functions I often use
#include "../src/muMidi.h"  //contains basic MIDI functions I often use
#include "../src/timer0.h"  //contains basic cpu based timer0 functions I often use

//STRUCTS
struct timer_t playbackTimer;
struct time_t time_data;

//GLOBALS
const char *midi_instruments[] = {
    "Ac. Grand Piano",	   "Bright Ac. Piano",	  "Electric Grand Piano",	"Honky-tonk Piano",
    "Electric Piano 1",	   "Electric Piano 2", 	  "Harpsichord", 		    "Clavinet",
    "Celesta",    		   "Glockenspiel",    	  "Music Box",    			"Vibraphone",
    "Marimba",   		   "Xylophone",    		  "Tubular Bells",    	 	"Santur",
    "Drawbar Organ",       "Percussive Organ",    "Rock Organ",    			"Church Organ",
    "Reed Organ",    	   "Accordion",    		  "Harmonica",    			"Tango Accordion",
    "Ac. Guitar (nylon)",  "Ac. Guitar (steel)",  "Elec. Guitar (jazz)",    "Elec. Guitar (clean)",
    "Elec. Guitar (muted)","Overdriven Guitar",   "Distortion Guitar",      "Guitar harmonics",
    "Acoustic Bass",       "Elec. Bass (finger)", "Elec. Bass (pick)",      "Fretless Bass",
    "Slap Bass 1",    	   "Slap Bass 2",    	  "Synth Bass 1",    		"Synth Bass 2",
    "Violin",    		   "Viola",    			  "Cello",   				"Contrabass",
    "Tremolo Strings",     "Pizzicato Strings",   "Orchestral Harp",    	"Timpani",
    "String Ensemble 1",   "String Ensemble 2",   "SynthStrings 1",    		"SynthStrings 2",
    "Choir Aahs",    	   "Voice Oohs",    	  "Synth Voice",    		"Orchestra Hit",
    "Trumpet",    		   "Trombone",    	      "Tuba",     				"Muted Trumpet",
    "French Horn",    	   "Brass Section",       "SynthBrass 1",    		"SynthBrass 2",
    "Soprano Sax",   	   "Alto Sax",    	      "Tenor Sax",    			"Baritone Sax",
    "Oboe",    	 	 	   "English Horn",   	  "Bassoon",    			"Clarinet",
    "Piccolo",    		   "Flute",    		      "Recorder",    			"Pan Flute",
    "Blown Bottle",    	   "Shakuhachi",    	  "Whistle",    			"Ocarina",
    "Lead 1 (square)",     "Lead 2 (sawtooth)",   "Lead 3 (calliope)",      "Lead 4 (chiff)",
    "Lead 5 (charang)",    "Lead 6 (voice)",      "Lead 7 (fifths)",    	"Lead 8 (bass + lead)",
    "Pad 1 (new age)",     "Pad 2 (warm)",        "Pad 3 (polysynth)",      "Pad 4 (choir)",
    "Pad 5 (bowed)",       "Pad 6 (metallic)",    "Pad 7 (halo)",    		"Pad 8 (sweep)",
    "FX 1 (rain)",         "FX 2 (soundtrack)",   "FX 3 (crystal)",    		"FX 4 (atmosphere)",
    "FX 5 (brightness)",   "FX 6 (goblins)",      "FX 7 (echoes)",    		"FX 8 (sci-fi)",
    "Sitar",    		   "Banjo",    			  "Shamisen",    			"Koto",
    "Kalimba",    		   "Bag pipe",    		  "Fiddle",    				"Shanai",
    "Tinkle Bell",    	   "Agogo",    		      "Steel Drums",    		"Woodblock",
    "Taiko Drum",    	   "Melodic Tom",         "Synth Drum",    			"Reverse Cymbal",
    "Guitar Fret Noise",   "Breath Noise",        "Seashore",    			"Bird Tweet",
    "Telephone Ring",      "Helicopter",          "Applause",    			"Gunshot"
};


//PROTOTYPES
uint8_t loadSMFile(char *, uint32_t);
int16_t getAndAnalyzeMIDI(struct midiRecord *, struct bigParsedEventList *);
void detectStructure(uint16_t, struct midiRecord *, struct bigParsedEventList *);
int16_t findPositionOfHeader(void);
void adjustOffsets(struct bigParsedEventList *);	
int8_t parse(uint16_t, bool, struct midiRecord *, struct bigParsedEventList *);
void playmidi(struct midiRecord *, struct bigParsedEventList *, bool);
uint32_t getTotalLeft(struct bigParsedEventList *);
void sendAME(aMEPtr, bool);
void displayInfo(struct midiRecord *);
void extraInfo(struct midiRecord *,struct bigParsedEventList *);
void superExtraInfo(struct midiRecord *);
void wipeStatus(void);
void midiPlaybackShimmering(uint8_t, uint8_t, bool);
void updateInstrumentDisplay(uint8_t, uint8_t);
void updateProgress(uint8_t);
void drawBorders(void);
byte bcdToDec(byte);

//FUNCTIONS

//this function will convert binary coded decimals into regular decimals
byte bcdToDec(byte bcd){
	uint16_t tens = (bcd >> 4) & 0xF;
	uint16_t units = bcd & 0xF;
	return LOW_BYTE(tens * 10 + units);
}

//sends a MIDI event message, either a 2-byte or 3-byte one
void sendAME(aMEPtr midiEvent, bool wantAlt)
	{
	POKE(wantAlt?MIDI_FIFO_ALT:MIDI_FIFO, midiEvent->msgToSend[0]);
	POKE(wantAlt?MIDI_FIFO_ALT:MIDI_FIFO, midiEvent->msgToSend[1]);
	if(midiEvent->bytecount == 3) POKE(wantAlt?MIDI_FIFO_ALT:MIDI_FIFO, midiEvent->msgToSend[2]);
	if((midiEvent->msgToSend[0] & 0xF0) == 0x90) 
			midiPlaybackShimmering(midiEvent->msgToSend[0] & 0x0F,  midiEvent->msgToSend[1], false);
	else if((midiEvent->msgToSend[0] & 0xF0) == 0x80)
			midiPlaybackShimmering(midiEvent->msgToSend[0] & 0x0F,  midiEvent->msgToSend[1], true);
	else if((midiEvent->msgToSend[0] & 0xF0) == 0xC0)
		 updateInstrumentDisplay(midiEvent->msgToSend[0] & 0x0F, midiEvent->msgToSend[1]);
	}
	
//Opens the std MIDI file
uint8_t loadSMFile(char *name, uint32_t targetAddress)
{
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
int16_t getAndAnalyzeMIDI(struct midiRecord *rec, struct bigParsedEventList *list)
	{		
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
int16_t findPositionOfHeader()
	{
	char targetSequence[] = "MThd";
    char *position;
    int16_t thePosition = 0;
	char buffer[64];
	uint16_t i=0;
	
	for(i=0;i<64;i++) buffer[i] = FAR_PEEK(MIDI_BASE + i);
	
    position = strstr(buffer, targetSequence);

	if(position != NULL)
		{
		thePosition = (int16_t)(position - buffer);	
		return thePosition;
		}
	return -1;
	}	

void adjustOffsets(struct bigParsedEventList *list)	 
{		
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
int8_t parse(uint16_t startIndex, bool wantCmds, struct midiRecord *rec, struct bigParsedEventList *list)
	{
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
     


void playmidi(struct midiRecord *rec, struct bigParsedEventList *list, bool vsorsam) //non-destructive version
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
	uint8_t progTrack=0;
	uint16_t progressSeconds=0;
	bool isPaused = false;
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
					sendAME(&msgGo, vsorsam);
					}
				else 
					{ //for all the rest which have a time delay
						overFlow = lowestTimeFound;
						while(overFlow > 0x00FFFFFF) //0x00FFFFFF is the max value of the timer0 we can do
						{
							setTimer0(0xFF,0xFF,0xFF);
							while(isTimer0Done()==0) 
								; //delay up to maximum of 0x00FFFFFF = 2/3rds of a second
							POKE(T0_PEND,0x10); //clear timer0 at 0x10
							overFlow = overFlow - 0x00FFFFFF; //reduce the max value one by one until there is a remainder smaller than the max amount
						}
						//do the last delay that's under 2/3rds of a second
						setTimer0((uint8_t)(overFlow&0x000000FF),
							  (uint8_t)((overFlow&0x0000FF00)>>8),
							  (uint8_t)((overFlow&0x00FF0000)>>16));
						while(isTimer0Done()==0)
							;				
						POKE(T0_PEND,0x10); //clear timer0 at 0x10
						sendAME(&msgGo, vsorsam);
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
		
	kernelNextEvent();

	if(kernelEventData.type == kernelEvent(key.PRESSED))
		{
			if(kernelEventData.key.raw == 146)
				{
				midiShutAllChannels(false);
				return;
				}
			if(kernelEventData.key.raw == 32)
			{
				if(isPaused==false)
				{
					midiShutAllChannels(false);
					isPaused = true;
					textSetColor(1,0);textGotoXY(26,24);textPrint("pause");
				}	
				else
				{
					isPaused = false;
					textSetColor(0,0);textGotoXY(26,24);textPrint("pause");				
				}

			}
		} // end if key pressed
	
	}//end of the whole playback
}//end function
	
void playmiditype0(struct midiRecord *rec, struct bigParsedEventList *list, bool vsorsam)
{
	uint16_t localTotalLeft=0;
	uint32_t whereTo; //in far memory, keeps adress of event to send
	uint32_t overFlow;
	aME msgGo;
	bool exitFlag = false;
	uint8_t progTrack=0;
	uint16_t progressSeconds=0;
	bool isPaused = false;
	byte oldsec=0;
	
	localTotalLeft = getTotalLeft(list);
	
	playbackTimer.absolute = getTimerAbsolute(TIMER_SECONDS)+1;
	setTimer(&playbackTimer);
	
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
			sendAME(&msgGo, vsorsam);
			
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
	kernelNextEvent();

	if(kernelEventData.type == kernelEvent(key.PRESSED))
		{
			if(kernelEventData.key.raw == 146)
			{
				midiShutAllChannels(false);
				return;
			}
			if(kernelEventData.key.raw == 32)
			{
				if(isPaused==false)
				{
					midiShutAllChannels(false);
					isPaused = true;
					textSetColor(1,0);textGotoXY(26,24);textPrint("pause");
				}	
				else
				{
					isPaused = false;
					textSetColor(0,0);textGotoXY(26,24);textPrint("pause");				
				}

			}
		} // end if key pressed
	
	}//end of the whole playback
}//end function

void wipeStatus()
{
	textGotoXY(0,25);textPrint("                                        ");
}

void displayInfo(struct midiRecord *rec)
{
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

void extraInfo(struct midiRecord *rec,struct bigParsedEventList *list)
{
	
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
	textGotoXY(0,25);printf("  ->Preparing for playback...");
}
void superExtraInfo(struct midiRecord *rec)
{
	uint16_t temp;
	
	temp=(uint32_t)((rec->totalDuration)/125000);
	temp=(uint32_t)((((double)temp))/((double)(rec->fudge)));
	rec->totalSec = temp;
	textGotoXY(68,5); printf("%d:%02d",temp/60,temp % 60);
	textGotoXY(1,24);textPrint("[ESC]: quit    [SPACE]:  pause");
}

void updateInstrumentDisplay(uint8_t chan, uint8_t pgr)
{
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
void initProgress()
{
	uint8_t i=0;
	textSetColor(15,0);textGotoXY(15,5);textPrint("[");
	for(i=0;i<51;i++) textPrint("_");
	textPrint("]");
}

void updateProgress(uint8_t prog)
{
	uint8_t i=0;
	textSetColor(5,0);textGotoXY(16,5);
	for(i=0;i<prog;i++) __putchar(18);
}
void midiPlaybackShimmering(uint8_t chan, uint8_t note, bool wantErase)
{
	textGotoXY(11+(note>>1),8+chan);textSetColor(chan+1,0);
	wantErase?__putchar(32):__putchar(42);
}
void setColors()
{
	uint8_t backup, i;
	backup = PEEK(MMU_IO_CTRL);
	POKE(MMU_IO_CTRL,0);
	POKE(0xD800, 0xFF);  //blue
	POKE(0xD801, 0xFF); //green
	POKE(0xD802, 0xFF); //red
	POKE(0xD803, 0);
	for(i=1;i<6;i++) //do yellows
	{
		POKE(0xD800+4*i,  0);              //blue
		POKE(0xD800+4*i+1, 0xCD + (i-1) * 10); //green
		POKE(0xD800+4*i+2, 0xCD + (i-1) * 10); //red
		POKE(0xD800+4*i+3, 0); 		     //unused alpha
	}
	for(i=6;i<11;i++) //do oranges
	{
		POKE(0xD800+4*i,  0);              //blue
		POKE(0xD800+4*i+1, (0xCD + (i-6) * 10)/2); //green
		POKE(0xD800+4*i+2, 0xCD + (i-6) * 10); //red
		POKE(0xD800+4*i+3, 0); 		     //unused alpha
	}
	for(i=11;i<16;i++) //do reds
	{
		POKE(0xD800+4*i,  0);              //blue
		POKE(0xD800+4*i+1,0); //green
		POKE(0xD800+4*i+2, 0xCD + (i-11) * 10); //red
		POKE(0xD800+4*i+3, 0); 		     //unused alpha
	}
	POKE(MMU_IO_CTRL, backup);
}
void drawBorders()
{
	
}
int main(int argc, char *argv[]) {
	bigParsed theBigList; //master structure to keep note of all tracks, nb of midi events, for playback
	midiRec myRecord; //keeps parsed info about the midi file, tempo, etc, for info display
	bool midiChip = false; //true = vs1053b, false=sam2695
	bool isDone = false; //to know when to exit the main loop; done playing
	int16_t indexStart = 0; //keeps note of the byte where the MIDI string 'MThd' is, sometimes they're not at the very start of the file!
	uint8_t i=0;
	
	//openAllCODEC(); //if the VS1053b is used, this might be necessary for some board revisions	
	//initVS1053MIDI();  //if the VS1053b is used

	wipeBitmapBackground(0x2F,0x2F,0x2F);
	POKE(MMU_IO_CTRL, 0x00);
	// XXX GAMMA  SPRITE   TILE  | BITMAP  GRAPH  OVRLY  TEXT
	POKE(VKY_MSTR_CTRL_0, 0b01000001); //sprite,graph,overlay,text
	// XXX XXX  FON_SET FON_OVLY | MON_SLP DBL_Y  DBL_X  CLK_70
	POKE(VKY_MSTR_CTRL_1, 0b00000100); //font overlay, double height text, 320x240 at 60 Hz;
	
	
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
	drawBorders();
	textGotoXY(0,25);printf("->Currently Loading file %s...",myRecord.fileName);
	resetInstruments(false); //resets all channels to piano, for sam2695
	midiShutUp(false); //ends trailing previous notes if any, for sam2695
	
	playbackTimer.units = TIMER_SECONDS;
	playbackTimer.cookie = TIMER_PLAYBACK_COOKIE;
	
	if(loadSMFile(myRecord.fileName, MIDI_BASE))
	{
		printf("\nCouldn't open %s\n",myRecord.fileName);
		printf("Press space to exit.");
		hitspace();
		return 0;
	}

	indexStart = getAndAnalyzeMIDI(&myRecord, &theBigList);

	displayInfo(&myRecord);
	
	setTimer0(0,0,0);
	if(indexStart!=-1) //found a place to start in the loaded file, proceed to play
		{
		parse(indexStart,false, &myRecord, &theBigList); //count the events and prep the mem allocation for the big list
		adjustOffsets(&theBigList);
		extraInfo(&myRecord,&theBigList);
		parse(indexStart,true, &myRecord, &theBigList); //load up the actual event data in the big list
		superExtraInfo(&myRecord);		
		wipeStatus();
		while(!isDone)
			{
			initProgress();
			if(myRecord.format == 0) playmiditype0(&myRecord, &theBigList, midiChip);
			else playmidi(&myRecord, &theBigList, midiChip);
			
			isDone=true;
			//reset the parsers
			for(i=0;i < theBigList.trackcount;i++)
				{
				myRecord.parsers[i] = 0;
				}
			}
		}	
	
	return 0;
	}
	