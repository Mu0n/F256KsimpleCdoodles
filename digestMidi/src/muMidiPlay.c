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
	kernelNextEvent();
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
void lilpause(uint8_t timedelay){
	struct timer_t pauseTimer;
	bool noteExitFlag = false;
	pauseTimer.units = 0; //frames
	pauseTimer.cookie = 213; //let's hope you never use this cookie!
	pauseTimer.absolute = getTimerAbsolute(0) + timedelay;
	setTimer(&pauseTimer);
	noteExitFlag = false;
	while(!noteExitFlag)
	{
		kernelNextEvent();
		if(kernelEventData.type == kernelEvent(timer.EXPIRED))
		{
			switch(kernelEventData.timer.cookie)
			{
			case 213:
				noteExitFlag = true;
				break;
			}
		}
	}
}
//writes the digested MIDI file format into a file
uint8_t writeDigestFile(char *name, struct midiRecord *rec, struct bigParsedEventList *theBigList) {
	FILE *fileID =0;
	uint16_t i, imax; //trackcount index
	uint16_t j, jmax; //eventcount index
	uint16_t k, kmax; //bytecount index
	uint8_t dummy; //dummy place to read into
	uint32_t tracker = 0x00000000; //will follow the far memory structure to parse it
	uint16_t delta = 0;
	uint8_t  cmdByte = 0;
	uint16_t tally =0;

	fileID = fileOpen(name,"w");
	printf("opened the file ID=%d with kernel\n",*fileID);

	tracker = rec->parsedAddr;
	//find this tune's track count and write it
	imax = theBigList->trackcount;
	jmax=theBigList->TrackEventList[0].eventcount;
	
	tally = fileWrite(&imax, sizeof(uint8_t), 2, fileID);
	kernelNextEvent();
	tracker+=(uint32_t)2;

	printf("there will be %d tracks\n",imax);	
	for(i=0;i<imax;i++) //loop over every track
		{
	  //find this track's event count and write it
		jmax=theBigList->TrackEventList[i].eventcount;
		printf("track %d count: %04x\n",i,jmax);
		fileWrite(&jmax,sizeof(uint8_t),2,fileID);
		kernelNextEvent();
		tracker += (uint32_t)2;
	  //
		for(j=0;j<jmax;j++) //loop over every event
			{
			 //find out the delta, it's 4 bytes
			delta = FAR_PEEKW(tracker);
			fileWrite(&delta,sizeof(uint8_t),2,fileID);
			kernelNextEvent();
			tracker +=(uint32_t)2;
			delta = FAR_PEEKW(tracker);
			fileWrite(&delta,sizeof(uint8_t),2,fileID);
			kernelNextEvent();
			tracker +=(uint32_t)2;
			
	  //find out if the command is 2 or 3 bytes
			kmax = FAR_PEEK(tracker);		
			fileWrite(&kmax,sizeof(uint8_t),1,fileID);
			kernelNextEvent();
			tracker+=(uint32_t)1;
			
			
			for(k=0; k<kmax; k++) //loop over every event byte
				{
			//read each byte of the command and write it
				cmdByte = FAR_PEEK(tracker);
				fileWrite(&cmdByte,1,1,fileID);
				kernelNextEvent();
				tracker+=(uint32_t)1;
				}
			if(kmax == 2) //top it off with a 0 on the last byte, to make every event a rounded 8 byte thing 
				{
				cmdByte = 0;
				fileWrite(&cmdByte,1,1,fileID);
				kernelNextEvent();
				tracker+=(uint32_t)1;
				}
			} //end loop event
		}//end every track
		
	fileClose(fileID);
	printf("closed the file with kernel, %04ld bytes written\n",tracker-rec->parsedAddr);

	hitspace();
	return 0;
	}

//reads the digested MIDI file format into a file
uint8_t readDigestFile(char *name, struct midiRecord *rec, struct bigParsedEventList *theBigList) {
	FILE *fileID =0;
	uint16_t i, imax; //trackcount index
	uint16_t j, jmax; //eventcount index
	uint16_t k, kmax; //bytecount index
	uint8_t dummy; //dummy place to read into
	uint32_t tracker = 0x00000000; //will follow the far memory structure to parse it
	uint16_t delta = 0, delta2 = 0;
	uint8_t  cmdByte = 0;
	uint16_t tally =0;

	fileID = fileOpen(name,"r");
	printf("opened the file ID=%d with kernel\n",*fileID);

	tracker = rec->parsedAddr;
	//find this tune's track count and write it
	fileRead(&imax,sizeof(uint8_t),2,fileID);
	tracker+=(uint32_t)2;
	
	printf("there will be %d tracks\n",imax);	
	
		for(i=0;i<imax;i++) //loop over every track
		{
		fileRead(&jmax,sizeof(uint8_t),2,fileID);
		tracker+=(uint32_t)2;
		printf("track %d count: %04x\n",i,jmax);
		for(j=0;j<jmax;j++) //loop over every event
			{
			fileRead(&delta,sizeof(uint8_t),2,fileID);
			fileRead(&delta2,sizeof(uint8_t),2,fileID);
			tracker += (uint32_t)4;
			
			
			fileRead(&kmax,sizeof(uint8_t),1,fileID);
			for(k=0; k<kmax; k++) //loop over every event byte
				{
			//read each byte of the command and write it
				fileRead(&cmdByte,sizeof(uint8_t),1,fileID);
				tracker+=(uint32_t)1;
				}
			if(kmax == 2) //top it off with a 0 on the last byte, to make every event a rounded 8 byte thing 
				{
				fileRead(&cmdByte,sizeof(uint8_t),1,fileID);
				tracker+=(uint32_t)1;
				}
			} //end loop event
		}//end every track
	
	/*
	jmax=theBigList->TrackEventList[0].eventcount;
	
	tally = fileWrite(&imax, sizeof(uint8_t), 2, fileID);
	kernelNextEvent();
	tracker+=(uint32_t)2;

	printf("there will be %d tracks\n",imax);	
	for(i=0;i<imax;i++) //loop over every track
		{
	  //find this track's event count and write it
		jmax=theBigList->TrackEventList[i].eventcount;
		printf("track %d count: %04x\n",i,jmax);
		fileWrite(&jmax,sizeof(uint8_t),2,fileID);
		kernelNextEvent();
		tracker += (uint32_t)2;
	  //
		for(j=0;j<jmax;j++) //loop over every event
			{
			 //find out the delta, it's 4 bytes
			delta = FAR_PEEKW(tracker);
			fileWrite(&delta,sizeof(uint8_t),2,fileID);
			kernelNextEvent();
			tracker +=(uint32_t)2;
			delta = FAR_PEEKW(tracker);
			fileWrite(&delta,sizeof(uint8_t),2,fileID);
			kernelNextEvent();
			tracker +=(uint32_t)2;
			
	  //find out if the command is 2 or 3 bytes
			kmax = FAR_PEEK(tracker);		
			fileWrite(&kmax,sizeof(uint8_t),1,fileID);
			kernelNextEvent();
			tracker+=(uint32_t)1;
			
			
			for(k=0; k<kmax; k++) //loop over every event byte
				{
			//read each byte of the command and write it
				cmdByte = FAR_PEEK(tracker);
				fileWrite(&cmdByte,1,1,fileID);
				kernelNextEvent();
				tracker+=(uint32_t)1;
				}
			if(kmax == 2) //top it off with a 0 on the last byte, to make every event a rounded 8 byte thing 
				{
				cmdByte = 0;
				fileWrite(&cmdByte,1,1,fileID);
				kernelNextEvent();
				tracker+=(uint32_t)1;
				}
			} //end loop event
		}//end every track
	*/	
	fileClose(fileID);
	printf("closed the file with kernel, %04ld bytes read\n",tracker-rec->parsedAddr);

	hitspace();
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
     	           (uint16_t) (FAR_PEEK(rec->baseAddr+i+1))
    			  |(uint16_t) (FAR_PEEK(rec->baseAddr+i)<<8)
    			  ;
    i+=2;

    rec->trackcount = 
     	           (uint16_t)(FAR_PEEK(rec->baseAddr+i+1))  
    			  |(uint16_t)((FAR_PEEK(rec->baseAddr+i)<<8)) 
    			  ;
    i+=2;

    rec->tick = (uint16_t)(
     	           (uint16_t)(FAR_PEEK(rec->baseAddr+i+1))  
    			  |(uint16_t)((FAR_PEEK(rec->baseAddr+i)<<8)) 
    			  );
    i+=2;
	
    currentTrack=0;
    
    while(currentTrack < rec->trackcount)
    	{	
		i+=4; //skip 'MTrk' header 4 character string
		trackLength =  (((uint32_t)(FAR_PEEK(rec->baseAddr+i)))<<24) 
		             | (((uint32_t)(FAR_PEEK(rec->baseAddr+i+1)))<<16)
					 | (((uint32_t)(FAR_PEEK(rec->baseAddr+i+2)))<<8)
					 |  ((uint32_t)(FAR_PEEK(rec->baseAddr+i+3)));
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
			
			nValue = (uint32_t)FAR_PEEK(rec->baseAddr+i);
			i++;
			if(nValue & 0x00000080)
				{
				nValue &= 0x0000007F;
				nValue <<= 7;
				nValue2 = (uint32_t)FAR_PEEK(rec->baseAddr+i);
				i++;
				if(nValue2 & 0x00000080)
					{
					nValue2 &= 0x0000007F;
					nValue2 <<= 7;
					nValue <<= 7;
					nValue3 = (uint32_t)FAR_PEEK(rec->baseAddr+i);
					i++;
					if(nValue3 & 0x00000080)
						{
						nValue3 &= 0x0000007F;
						nValue3 <<= 7;
						nValue2 <<= 7;
						nValue <<= 7;
						nValue4 = (uint32_t)FAR_PEEK(rec->baseAddr+i);
						i++;
						} //end of getting to nValue4
					} //end of getting to nValue3
				} //end of getting to nValue2	
    		timeDelta = nValue | nValue2 | nValue3 | nValue4;
			

    		//status byte or MIDI message reading
    		status_byte = FAR_PEEK(rec->baseAddr+i);
			extra_byte  = FAR_PEEK(rec->baseAddr+i+1); //be ready for 2 bytes midi events
			extra_byte2 = FAR_PEEK(rec->baseAddr+i+2); //be ready for 3 bytes midi events
			i++;
			
			lastCmdPreserver = false;
			//first, check for run-on commands that don't repeat the status_byte
			if(status_byte < 0x80)
				{
				i--;//go back 1 spot so it can read the data properly
				status_byte = last_cmd;					
				extra_byte  = FAR_PEEK(rec->baseAddr+i); //recycle the byte to its proper destination
				extra_byte2 = FAR_PEEK(rec->baseAddr+i+1); //redo: be ready for 3 bytes midi events
				 			
				}
			//second, deal with MIDI meta-event commands that start with 0xFF
			if(status_byte == 0xFF)
				{
				meta_byte = (uint8_t)FAR_PEEK(rec->baseAddr+i);
				i++;
				if(meta_byte == MetaSequence)
					{
					i+=2;
					}
				else if(meta_byte == MetaText)
					{
					data_byte = (uint8_t)FAR_PEEK(rec->baseAddr+i); //length of text
					i+=(uint16_t)data_byte + 1;
					}
				else if(meta_byte == MetaCopyright)
					{
					data_byte = (uint8_t)FAR_PEEK(rec->baseAddr+i); //length of text
					i+=(uint16_t)data_byte + 1;
					}
				else if(meta_byte == MetaTrackName)
					{
					data_byte = (uint8_t)FAR_PEEK(rec->baseAddr+i); //length of text
					i+=(uint16_t)data_byte + 1;
					}
				else if(meta_byte == MetaInstrumentName)
					{
					data_byte = (uint8_t)FAR_PEEK(rec->baseAddr+i); //length of text
					i+=(uint16_t)data_byte + 1;
					}
				else if(meta_byte == MetaLyrics)
					{
					data_byte = (uint8_t)FAR_PEEK(rec->baseAddr+i); //length of text
					i+=(uint16_t)data_byte + 1;
					}
				else if(meta_byte == MetaMarker)
					{
					data_byte = (uint8_t)FAR_PEEK(rec->baseAddr+i); //length of text
					i+=(uint16_t)data_byte + 1;
					}
				else if(meta_byte == MetaCuePoint)
					{
					data_byte = (uint8_t)FAR_PEEK(rec->baseAddr+i); //length of text
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
					data_byte = (uint8_t)FAR_PEEK(rec->baseAddr+i);
					i++;
					data_byte2 = (uint8_t)FAR_PEEK(rec->baseAddr+i);
					i++;
					data_byte3 = (uint8_t)FAR_PEEK(rec->baseAddr+i);
					i++;
					data_byte4 = (uint8_t)FAR_PEEK(rec->baseAddr+i);
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
					
					FAR_POKEW((uint32_t) rec->parsedAddr + (uint32_t) whereTo                , (uint16_t)((tempCalc & 0xFFFF0000)  >>16) ) ;
					FAR_POKEW((uint32_t) rec->parsedAddr + (uint32_t) whereTo + (uint32_t) 2 , (uint16_t)(tempCalc & 0x0000FFFF  ) ) ;
				
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
					FAR_POKEW((uint32_t) rec->parsedAddr + (uint32_t) whereTo                , (uint16_t)((tempCalc & 0xFFFF0000)  >>16) ) ;
					FAR_POKEW((uint32_t) rec->parsedAddr + (uint32_t) whereTo + (uint32_t) 2 , (uint16_t)(tempCalc & 0x0000FFFF  ) ) ;
				
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
     
	
uint8_t playmiditype0(struct midiRecord *rec, struct bigParsedEventList *list) {
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
			
			msgGo.deltaToGo =  (((uint32_t)((FAR_PEEKW((uint32_t) rec->parsedAddr + (uint32_t) whereTo))) << 16)
							 |   (uint32_t) (FAR_PEEKW((uint32_t) rec->parsedAddr + (uint32_t) whereTo + (uint32_t) 2)));
			msgGo.bytecount = 	FAR_PEEK((uint32_t) rec->parsedAddr + (uint32_t) whereTo + (uint32_t) AME_BYTECOUNT);
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
return 0;}
}//end function