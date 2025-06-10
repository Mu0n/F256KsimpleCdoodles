#include "D:\F256\llvm-mos\code\digestMidi\.builddir\trampoline.h"

#include "f256lib.h"
#include "../src/muMidiPlay.h"
#include "../src/muMidi.h"

//sends a MIDI event message, either a 2-byte or 3-byte one
void sendAME(aMEPtr midiEvent, bool wantAlt) {
	POKE(wantAlt?MIDI_FIFO_ALT:MIDI_FIFO, midiEvent->msgToSend[0]);
	POKE(wantAlt?MIDI_FIFO_ALT:MIDI_FIFO, midiEvent->msgToSend[1]);
	if(midiEvent->bytecount == 3) POKE(wantAlt?MIDI_FIFO_ALT:MIDI_FIFO, midiEvent->msgToSend[2]);
	}
	
//Opens the std MIDI file
uint8_t loadSMFile(char *name, uint32_t targetAddress) {
	FILE *theMIDIfile;
	uint8_t buffer[255];
	size_t bytesRead = 0;
	uint32_t totalBytesRead = 0;
	uint16_t i=0;

	theMIDIfile = fileOpen(name,"r"); // open file in read mode
	if(theMIDIfile == NULL) {
		return 1;
		}

	while ((bytesRead = fileRead(buffer, sizeof(uint8_t), 250, theMIDIfile))>0) {
			buffer[0]=buffer[0];
			//dump the buffer into a special RAM area
			for(i=0;i<bytesRead;i++)
				{
				FAR_POKE((uint32_t)targetAddress+(uint32_t)totalBytesRead+(uint32_t)i,buffer[i]);
				}
			totalBytesRead += (uint32_t) bytesRead;
			if(bytesRead < 250) break;
			}
	fileClose(theMIDIfile);
	return 0;
}

	
//high level function that directs the reading and parsing of the MIDI file
int16_t getAndAnalyzeMIDI(struct midiRecord *rec, struct bigParsedEventList *list) {
	int16_t indexToStart=0; //MThd should be at position 0, but it might not, so we'll find it
	indexToStart = findPositionOfHeader(rec->baseAddr); //find the start index of 'MThd'
	if(indexToStart == -1)
		{
		return -1;
		}
	detectStructure(indexToStart, rec, list); //parse it a first time to get the format type and nb of tracks
	return indexToStart;
	}

//checks the tempo, number of tracks, etc
void detectStructure(uint16_t startIndex, struct midiRecord *rec, struct bigParsedEventList *list) {
    uint32_t trackLength = 0; //size in bytes of the current track
    uint32_t i = startIndex; // #main array parsing index
    uint32_t j=0;
    uint16_t currentTrack=0; //index for the current track
	
    i+=4; //skip header tag
	i+=4; //skip SIZE which is always 6 anyway

    rec->format =
     	           (uint16_t) (FAR_PEEK(rec->baseAddr+i+1))
    			  |(uint16_t) (FAR_PEEK(rec->baseAddr+i)<<8)
    			  ;
    i+=2;
 
    rec->trackcount =
     	           (uint16_t)(FAR_PEEK(rec->baseAddr+i+1))
    			  |(uint16_t)((FAR_PEEK(rec->baseAddr+i)<<8))
    			  ;
    i+=2;
 
    rec->tick =
     	           (uint16_t)(FAR_PEEK(rec->baseAddr+i+1))
    			  |(uint16_t)((FAR_PEEK(rec->baseAddr+i)<<8))
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
	
	    	trackLength =  (((uint32_t)(FAR_PEEK(rec->baseAddr+i)))<<24)
		             | (((uint32_t)(FAR_PEEK(rec->baseAddr+i+1)))<<16)
					 | (((uint32_t)(FAR_PEEK(rec->baseAddr+i+2)))<<8)
					 |  ((uint32_t)(FAR_PEEK(rec->baseAddr+i+3)));
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
int findPositionOfHeader(uint32_t baseAddr) {
	char targetSequence[] = "MThd";
    char *position;
    int thePosition = 0;
	char buffer[64];
	int i=0;
	
	for(i=0;i<64;i++) buffer[i] = FAR_PEEK(baseAddr + i);
	
    position = strstr(buffer, targetSequence);

	if(position != NULL)
		{
		thePosition = (int)(position - buffer);
		return thePosition;
		}
	return -1;
	}

void adjustOffsets(struct bigParsedEventList *list) {
	uint16_t i=0,k=0;
	uint32_t currentEventCount=0;
	
	for(i=0;i<list->trackcount;i++)
	{
		list->TrackEventList[i].baseOffset = (uint32_t)0;
		for(k=0;k<i;k++) //do this for all tracks before it
			{
			currentEventCount=(uint32_t)list->TrackEventList[k].eventcount;
			list->TrackEventList[i].baseOffset += (currentEventCount*(uint32_t)MIDI_EVENT_FAR_SIZE ); //skip to a previous track
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
     	           (((uint16_t) FAR_PEEK(rec->baseAddr+(uint32_t)i+(uint32_t)1))
    			  | ((uint16_t) FAR_PEEK(rec->baseAddr+(uint32_t)i            ))<<8)
    			  ;
    i+=2;

    rec->trackcount =
     	           ((uint16_t)(FAR_PEEK(rec->baseAddr+(uint32_t)i+(uint32_t)1)))
    			  |(((uint16_t)FAR_PEEK(rec->baseAddr+(uint32_t)i))<<8)
    			  ;
    i+=2;

    rec->tick = (uint16_t)(
     	           ((uint16_t)(FAR_PEEK(rec->baseAddr+(uint32_t)i+(uint32_t)1)))
    			  |(((uint16_t)FAR_PEEK(rec->baseAddr+(uint32_t)i))<<8)
    			  );
    i+=2;
	
    currentTrack=0;
 
    while(currentTrack < rec->trackcount)
    	{
		i+=4; //skip 'MTrk' header 4 character string
		trackLength =  ((((uint32_t)FAR_PEEK(rec->baseAddr+(uint32_t)i            )))<<24)
		             | ((((uint32_t)FAR_PEEK(rec->baseAddr+(uint32_t)i+(uint32_t)1)))<<16)
					 | ((((uint32_t)FAR_PEEK(rec->baseAddr+(uint32_t)i+(uint32_t)2)))<<8 )
					 | ((((uint32_t)FAR_PEEK(rec->baseAddr+(uint32_t)i+(uint32_t)3)))    );
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
	
			nValue = (uint32_t)FAR_PEEK(rec->baseAddr+(uint32_t)i);
			i++;
			if(nValue & 0x00000080)
				{
				nValue &= 0x0000007F;
				nValue <<= 7;
				nValue2 = (uint32_t)FAR_PEEK(rec->baseAddr+(uint32_t)i);
				i++;
				if(nValue2 & 0x00000080)
					{
					nValue2 &= 0x0000007F;
					nValue2 <<= 7;
					nValue <<= 7;
					nValue3 = (uint32_t)FAR_PEEK(rec->baseAddr+(uint32_t)i);
					i++;
					if(nValue3 & 0x00000080)
						{
						nValue3 &= 0x0000007F;
						nValue3 <<= 7;
						nValue2 <<= 7;
						nValue <<= 7;
						nValue4 = (uint32_t)FAR_PEEK(rec->baseAddr+(uint32_t)i);
						i++;
						} //end of getting to nValue4
					} //end of getting to nValue3
				} //end of getting to nValue2
    		timeDelta = nValue | nValue2 | nValue3 | nValue4;
	

    		//status byte or MIDI message reading
    		status_byte = FAR_PEEK(rec->baseAddr+(uint32_t)i);
			extra_byte  = FAR_PEEK(rec->baseAddr+(uint32_t)i+(uint32_t)1); //be ready for 2 bytes midi events
			extra_byte2 = FAR_PEEK(rec->baseAddr+(uint32_t)i+(uint32_t)2); //be ready for 3 bytes midi events
			i++;
	
			lastCmdPreserver = false;
			//first, check for run-on commands that don't repeat the status_byte
			if(status_byte < 0x80)
				{
				i--;//go back 1 spot so it can read the data properly
				status_byte = last_cmd;
				extra_byte  = FAR_PEEK(rec->baseAddr+(uint32_t)i); //recycle the byte to its proper destination
				extra_byte2 = FAR_PEEK(rec->baseAddr+(uint32_t)i+(uint32_t)1); //redo: be ready for 3 bytes midi events
	
				}
			//second, deal with MIDI meta-event commands that start with 0xFF
			if(status_byte == 0xFF)
				{
				meta_byte = FAR_PEEK(rec->baseAddr+(uint32_t)i);
				i++;
				if(meta_byte == MetaSequence || meta_byte == MetaChannelPrefix || meta_byte == MetaChangePort)
					{
					i+=(uint32_t)2;
					}
				else if(meta_byte == MetaText || meta_byte == MetaCopyright || meta_byte == MetaTrackName || meta_byte == MetaInstrumentName
				         || meta_byte == MetaLyrics || meta_byte == MetaMarker || meta_byte == MetaCuePoint)
					{
					data_byte = (uint8_t)FAR_PEEK(rec->baseAddr+i); //length of text
					i+=(uint32_t)data_byte + (uint32_t)1;
					}
				else if(meta_byte == MetaEndOfTrack)
					{
					i++;
					continue;
					}
				else if(meta_byte == MetaSetTempo)
					{
					data_byte =  FAR_PEEK(rec->baseAddr+(uint32_t)i);
					i++;
					data_byte2 = FAR_PEEK(rec->baseAddr+(uint32_t)i);
					i++;
					data_byte3 = FAR_PEEK(rec->baseAddr+(uint32_t)i);
					i++;
					data_byte4 = FAR_PEEK(rec->baseAddr+(uint32_t)i);
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
					rec->nn = (uint8_t)FAR_PEEK(rec->baseAddr+i);
					i++;
					rec->dd = (uint8_t)FAR_PEEK(rec->baseAddr+i);
					i++;
					rec->cc = (uint8_t)FAR_PEEK(rec->baseAddr+i);
					i++;
					rec->bb = (uint8_t)FAR_PEEK(rec->baseAddr+i);
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
	
					FAR_POKE((uint32_t) rec->parsedAddr + (uint32_t) whereTo                , (uint8_t)((tempCalc & 0x000000FF)      ) ) ;
					FAR_POKE((uint32_t) rec->parsedAddr + (uint32_t) whereTo + (uint32_t) 1 , (uint8_t)((tempCalc & 0x0000FF00)  >>8 ) ) ;
					FAR_POKE((uint32_t) rec->parsedAddr + (uint32_t) whereTo + (uint32_t) 2 , (uint8_t)((tempCalc & 0x00FF0000)  >>16) ) ;
					FAR_POKE((uint32_t) rec->parsedAddr + (uint32_t) whereTo + (uint32_t) 3 , (uint8_t)((tempCalc & 0xFF000000)  >>24) ) ;
	
	
					FAR_POKE((uint32_t) rec->parsedAddr + (uint32_t) whereTo + (uint32_t) AME_BYTECOUNT, 0x02);
					FAR_POKE((uint32_t) rec->parsedAddr + (uint32_t) whereTo + (uint32_t) AME_MSG, status_byte);
					FAR_POKE((uint32_t) rec->parsedAddr + (uint32_t) whereTo + (uint32_t) AME_MSG+(uint32_t) 1, extra_byte);
					FAR_POKE((uint32_t) rec->parsedAddr + (uint32_t) whereTo + (uint32_t) AME_MSG+(uint32_t) 2, 0x00); //not needed but meh
	
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
					FAR_POKE((uint32_t) rec->parsedAddr + (uint32_t) whereTo                , (uint8_t)((tempCalc & 0x000000FF)      ) ) ;
					FAR_POKE((uint32_t) rec->parsedAddr + (uint32_t) whereTo + (uint32_t) 1 , (uint8_t)((tempCalc & 0x0000FF00)  >>8 ) ) ;
					FAR_POKE((uint32_t) rec->parsedAddr + (uint32_t) whereTo + (uint32_t) 2 , (uint8_t)((tempCalc & 0x00FF0000)  >>16) ) ;
					FAR_POKE((uint32_t) rec->parsedAddr + (uint32_t) whereTo + (uint32_t) 3 , (uint8_t)((tempCalc & 0xFF000000)  >>24) ) ;
	
	
					FAR_POKE((uint32_t) rec->parsedAddr + (uint32_t) whereTo + (uint32_t) AME_BYTECOUNT, 0x03);
					FAR_POKE((uint32_t) rec->parsedAddr + (uint32_t) whereTo + (uint32_t) AME_MSG, status_byte);
					FAR_POKE((uint32_t) rec->parsedAddr + (uint32_t) whereTo + (uint32_t) AME_MSG+(uint32_t) 1, extra_byte);
					FAR_POKE((uint32_t) rec->parsedAddr + (uint32_t) whereTo + (uint32_t) AME_MSG+(uint32_t) 2, extra_byte2);
	
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
 
//writes the digested MIDI file format into a file
uint8_t writeDigestFile(char *name, struct midiRecord *rec, struct bigParsedEventList *theBigList) {
	
	FILE *fileID =0;
	uint16_t i, imax; //trackcount index
	uint16_t j; //eventcount index
	uint16_t k, kmax; //bytecount index
	uint32_t tracker = 0x00000000; //will follow the far memory structure to parse it
	uint8_t delta1, delta2;
	uint8_t  cmdByte = 0;

	fileID = fileOpen(name,"w");
	
	//set tracker
	tracker = rec->parsedAddr;
	
	//find this tune's track count and write it
	imax = theBigList->trackcount;
	
	fileWrite(&imax, sizeof(uint16_t), 1, fileID);
	kernelNextEvent();

	//first pass and write track event counts


	for(i=0;i<imax;i++) //loop over every track
		{
	  //find this track's event count and write it
		fileWrite(&(theBigList->TrackEventList[i].eventcount),sizeof(uint16_t),1,fileID);
	kernelNextEvent();
		}

	for(i=0;i<imax;i++) //loop over every track, now concentrate on their data
		{
	
		for(j=0;j<theBigList->TrackEventList[i].eventcount;j++) //loop over every event
			{
		    //find out the delta, it's 4 bytes
			delta1 = FAR_PEEK(tracker);
			tracker +=(uint32_t)1;
			fileWrite(&delta1,sizeof(uint8_t),1,fileID);
			delta1 = FAR_PEEK(tracker);
			tracker +=(uint32_t)1;
			fileWrite(&delta1,sizeof(uint8_t),1,fileID);
	
			delta2 = FAR_PEEK(tracker);
			tracker +=(uint32_t)1;
			fileWrite(&delta2,sizeof(uint8_t),1,fileID);
			delta2 = FAR_PEEK(tracker);
			tracker +=(uint32_t)1;
			fileWrite(&delta2,sizeof(uint8_t),1,fileID);
	
	  //find out if the command is 2 or 3 bytes
			kmax = FAR_PEEK(tracker);
			fileWrite(&kmax,sizeof(uint8_t),1,fileID);
			tracker+=(uint32_t)1;
	
	
			for(k=0; k<kmax; k++) //loop over every event byte
				{
			//read each byte of the command and write it
				cmdByte = FAR_PEEK(tracker);
				fileWrite(&cmdByte,sizeof(uint8_t),1,fileID);
				tracker+=(uint32_t)1;
				}
			if(kmax == 2) //top it off with a 0 on the last byte, to make every event a rounded 8 byte thing
				{
				cmdByte = 0;
				fileWrite(&cmdByte,sizeof(uint8_t),1,fileID);
				tracker+=(uint32_t)1;
				}
			} //end loop event
		} //end loop track

	fileClose(fileID);
	textGotoXY(20,3);printf("%04ld bytes written to %s\n",tracker-rec->parsedAddr, name);
	
	return 0;
	}


//reads the digested MIDI file format into a file
uint8_t readDigestFile(char *name, struct midiRecord *rec, struct bigParsedEventList *theBigList) {
	FILE *fileID =0;
	uint8_t buffer[255];
	size_t bytesRead = 0;
	uint32_t totalBytesRead = 0;
	uint16_t i; //trackcount index
	uint32_t readChunk = 0;
	uint32_t leftToRead = 0;
	uint8_t read1, read2;
	uint16_t index = 0;
	

	fileID = fileOpen(name,"r"); // open file in read mode
	if(fileID == NULL) {
		return 1;
		}

	fileRead(&read1,sizeof(uint8_t),1,fileID);
	fileRead(&read2,sizeof(uint8_t),1,fileID);
	theBigList->trackcount = (uint16_t)(read1) | ((((uint16_t)read2)<<8)&0xFF00); //get track count
	rec->trackcount = theBigList->trackcount;
	
	if(theBigList->TrackEventList != NULL) free(theBigList->TrackEventList); //assign structure in aTOEPtr
	theBigList->TrackEventList = (aTOEPtr) malloc((sizeof(aTOE)) * theBigList->trackcount);
	if(rec->parsers !=NULL) free(rec->parsers);
	rec->parsers = (uint16_t *) malloc(sizeof(uint16_t) * theBigList->trackcount);
	for(index=0;index<theBigList->trackcount;index++) //get event count for each track
	{
		if(index==theBigList->trackcount)break;
		rec->parsers[index]=0;
	
		fileRead(&read1,sizeof(uint8_t),1,fileID);
		fileRead(&read2,sizeof(uint8_t),1,fileID);
		theBigList->TrackEventList[index].eventcount = (uint16_t)(read1) | ((((uint16_t)read2)<<8)&0xFF00);//get track count
		theBigList->TrackEventList[index].trackno = index;
		theBigList->TrackEventList[index].baseOffset = 0;
	}
	adjustOffsets(&(*theBigList));

	for(uint16_t a=0;a<theBigList->trackcount;a++) //read the main portion of the data and put it in far memory
	{
		leftToRead = (uint32_t)theBigList->TrackEventList[a].eventcount * (uint32_t)MIDI_EVENT_FAR_SIZE;
		if(leftToRead == 0) continue;
		readChunk = 250;
		if(readChunk > leftToRead) readChunk = leftToRead;
		while(leftToRead > 0)
		{
			bytesRead = fileRead(buffer, sizeof(uint8_t), readChunk, fileID);
	
			for(i=0;i<bytesRead;i++)
				{
				FAR_POKE((uint32_t)rec->parsedAddr+(uint32_t)totalBytesRead+(uint32_t)i,buffer[i]);
				}
			totalBytesRead += (uint32_t) bytesRead;
			leftToRead -= readChunk;
	
			if(readChunk > leftToRead) readChunk = leftToRead;
		}
	}
	fileClose(fileID);
	return 0;
	}
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
		/*
		soundBeholders[i] =  ((((uint32_t)(FAR_PEEKW((uint32_t) rec->parsedAddr  + (uint32_t) whereTo))) << 16)
							|  (uint32_t) (FAR_PEEKW((uint32_t) rec->parsedAddr  + (uint32_t) whereTo + (uint32_t) 2)));
	
				*/
		soundBeholders[i] =  (
								((  (uint32_t)( FAR_PEEK((uint32_t) rec->parsedAddr + (uint32_t) whereTo                   ) ) )&0x000000FF)
							 |  ((  (uint32_t)( FAR_PEEK((uint32_t) rec->parsedAddr + (uint32_t) whereTo + (uint32_t)1) ) << 8 )&0x0000FF00)
							 |  ((  (uint32_t)( FAR_PEEK((uint32_t) rec->parsedAddr + (uint32_t) whereTo + (uint32_t)2) ) << 16)&0x00FF0000)
							 |  ((  (uint32_t)( FAR_PEEK((uint32_t) rec->parsedAddr + (uint32_t) whereTo + (uint32_t)3) ) << 24)&0xFF000000)
							 );
	}
	while(localTotalLeft > 0 && !exitFlag)
		{
		lowestTimeFound = 0xFFFFFFFF; //make it easy to find lower than this
	
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
					msgGo.bytecount =    FAR_PEEK((uint32_t) rec->parsedAddr + (uint32_t) whereToLowest + (uint32_t) AME_BYTECOUNT);
					msgGo.msgToSend[0] = FAR_PEEK((uint32_t) rec->parsedAddr + (uint32_t) whereToLowest + (uint32_t) AME_MSG);
					msgGo.msgToSend[1] = FAR_PEEK((uint32_t) rec->parsedAddr + (uint32_t) whereToLowest + (uint32_t) AME_MSG+(uint32_t) 1);
					msgGo.msgToSend[2] = FAR_PEEK((uint32_t) rec->parsedAddr + (uint32_t) whereToLowest + (uint32_t) AME_MSG+(uint32_t) 2);
	
				if(lowestTimeFound==0) //do these 0 delay events right away, no need to involve a Time Manager
					{
					sendAME(&msgGo, 0);
					}
				else
					{ //for all the rest which have a time delay
						overFlow = lowestTimeFound;
						while(overFlow > 0x00FFFFFF) //0x00FFFFFF is the max value of the timer0 we can do
						{
							setTimer0(0xFF,0xFF,0xFF);
							while(isTimer0Done()==0)
								;
								//delay up to maximum of 0x00FFFFFF = 2/3rds of a second
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
						sendAME(&msgGo, 0);
					}
	
				//Advance the marker for the track that just did something
				rec->parsers[lowestTrack]+=1;
				whereToLowest  = (uint32_t)(list->TrackEventList[lowestTrack].baseOffset);
				whereToLowest += (uint32_t)((uint32_t) rec->parsers[lowestTrack] *(uint32_t)  MIDI_EVENT_FAR_SIZE);
				//replenish the new delta here
				/*
				soundBeholders[lowestTrack] =    (((((uint32_t)FAR_PEEKW((uint32_t) rec->parsedAddr  + (uint32_t) whereToLowest))) << 16)
												| (uint32_t) (FAR_PEEKW((uint32_t) rec->parsedAddr  + (uint32_t) whereToLowest + (uint32_t) 2)));
				*/
	
				soundBeholders[lowestTrack] = (
								((  (uint32_t)( FAR_PEEK((uint32_t) rec->parsedAddr + (uint32_t) whereToLowest                   ) ) )&0x000000FF)
							 |  ((  (uint32_t)( FAR_PEEK((uint32_t) rec->parsedAddr + (uint32_t) whereToLowest + (uint32_t)1) ) << 8 )&0x0000FF00)
							 |  ((  (uint32_t)( FAR_PEEK((uint32_t) rec->parsedAddr + (uint32_t) whereToLowest + (uint32_t)2) ) << 16)&0x00FF0000)
							 |  ((  (uint32_t)( FAR_PEEK((uint32_t) rec->parsedAddr + (uint32_t) whereToLowest + (uint32_t)3) ) << 24)&0xFF000000)
							 );
				for(i=0;i<trackcount;i++)
				{
					if(rec->parsers[i] >= (list->TrackEventList[i].eventcount)) continue;//that track was already exhausted
					if(i==lowestTrack) continue; //don't mess with the track that just acted
					soundBeholders[i] -= lowestTimeFound;
				}
				localTotalLeft--;
	
	}//end of the whole playback
	return 0;
}//end
	
uint8_t playmiditype0(struct midiRecord *rec, struct bigParsedEventList *list)
{
	uint16_t localTotalLeft=0;
	uint32_t whereTo; //in far memory, keeps adress of event to send
	uint32_t overFlow;
	aME msgGo;
	bool exitFlag = false;
	
	localTotalLeft = getTotalLeft(list);
	
	while(localTotalLeft > 0 && !exitFlag)
	{
			whereTo  = (uint32_t)(list->TrackEventList[0].baseOffset);
			whereTo += (uint32_t) ((uint32_t)rec->parsers[0] * (uint32_t) MIDI_EVENT_FAR_SIZE);
	
			msgGo.deltaToGo =  (
								((  (uint32_t)( FAR_PEEK((uint32_t) rec->parsedAddr + (uint32_t) whereTo                   ) ) )&0x000000FF)
							 |  ((  (uint32_t)( FAR_PEEK((uint32_t) rec->parsedAddr + (uint32_t) whereTo + (uint32_t)1) ) << 8 )&0x0000FF00)
							 |  ((  (uint32_t)( FAR_PEEK((uint32_t) rec->parsedAddr + (uint32_t) whereTo + (uint32_t)2) ) << 16)&0x00FF0000)
							 |  ((  (uint32_t)( FAR_PEEK((uint32_t) rec->parsedAddr + (uint32_t) whereTo + (uint32_t)3) ) << 24)&0xFF000000)
							 );
			msgGo.bytecount    = FAR_PEEK((uint32_t) rec->parsedAddr + (uint32_t) whereTo + (uint32_t) AME_BYTECOUNT);
			msgGo.msgToSend[0] = FAR_PEEK((uint32_t) rec->parsedAddr + (uint32_t) whereTo + (uint32_t) AME_MSG);
			msgGo.msgToSend[1] = FAR_PEEK((uint32_t) rec->parsedAddr + (uint32_t) whereTo + (uint32_t) AME_MSG+1);
			msgGo.msgToSend[2] = FAR_PEEK((uint32_t) rec->parsedAddr + (uint32_t) whereTo + (uint32_t) AME_MSG+2);
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
			sendAME(&msgGo, 0);
			rec->parsers[0]++;
			localTotalLeft--;

	}//end of the whole playback
return 0;
}
