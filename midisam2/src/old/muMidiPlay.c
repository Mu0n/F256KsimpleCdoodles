#include "f256lib.h"
#include "../src/muMidiPlay.h"
#include "../src/muMidi.h"

uint8_t checkKeySkip()
{
	kernelNextEvent();
	if(kernelEventData.type == kernelEvent(key.PRESSED)) return 1;
	return 0;
}

//sends a MIDI event message, either a 2-byte or 3-byte one
void sendAME(aMEPtr midiEvent, bool wantAlt) {
	POKE(wantAlt?MIDI_FIFO_ALT:MIDI_FIFO, midiEvent->msgToSend[0]);
	POKE(wantAlt?MIDI_FIFO_ALT:MIDI_FIFO, midiEvent->msgToSend[1]);
	if(midiEvent->bytecount == 3) POKE(wantAlt?MIDI_FIFO_ALT:MIDI_FIFO, midiEvent->msgToSend[2]);
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

//reads an embedded .DIM file from RAM and plays it asynchronously
void playEmbeddedDim(uint32_t parsedAddr)
{
	uint16_t nbTracks=0; //read off the number of tracks found in the header, very first 2 bytes at parsedAddr
	uint32_t tracker = parsedAddr;
	
	bigParsed theBigList; //master structure to keep note of all tracks, nb of midi events, for playback
	midiRec myRecord; //keeps parsed info about the midi file, tempo, etc, for info display
	initBigList(&theBigList);
	
	//deal with number of tracks
	nbTracks = ((uint16_t)FAR_PEEK(tracker)) | ((((uint16_t)FAR_PEEK(tracker+1))<<8));
	tracker += 2;
	theBigList.trackcount = nbTracks;
	theBigList.TrackEventList = (aTOEPtr) malloc((sizeof(aTOE)) * theBigList.trackcount);
	
	
	for(uint8_t i=0;i<nbTracks;i++)
	{
		theBigList.TrackEventList[i].eventcount =  ( (uint16_t)FAR_PEEK(tracker    )   ) 
												 | ( (uint16_t)FAR_PEEK(tracker + 1)<<8);
		tracker+=2;
	}
	adjustOffsets(&theBigList);
	initMidiRecord(&myRecord, 0, tracker); //we don't need the middle argument, there's no SMF embedded data
	if(myRecord.parsers !=NULL) free(myRecord.parsers);
	myRecord.parsers = (uint16_t *) malloc(sizeof(uint16_t) * theBigList.trackcount);

//	printf("track count %d\n",theBigList.trackcount);
	for(uint8_t i=0;i<nbTracks;i++) 
	{
		myRecord.parsers[i]=0;
//		printf("t%d e%d\n",i,theBigList.TrackEventList[i].eventcount);
//		printf("%08lx\n",theBigList.TrackEventList[i].baseOffset);
	}
//	printf("%08lx",tracker);
	if(theBigList.trackcount == 1) playmiditype0(&myRecord, &theBigList);
	else playmidi(&myRecord, &theBigList);
}


//reads the digested MIDI file format into RAM
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
					if(checkKeySkip()) return 0;
					}
				else 
					{ //for all the rest which have a time delay
						overFlow = lowestTimeFound;
						while(overFlow > 0x00FFFFFF) //0x00FFFFFF is the max value of the timer0 we can do
						{
							setTimer0(0xFF,0xFF,0xFF);
							while(isTimer0Done()==0) if(checkKeySkip()) return 0;
								//delay up to maximum of 0x00FFFFFF = 2/3rds of a second
							POKE(T0_PEND,0x10); //clear timer0 at 0x10
							overFlow = overFlow - 0x00FFFFFF; //reduce the max value one by one until there is a remainder smaller than the max amount
						}
						//do the last delay that's under 2/3rds of a second
						setTimer0((uint8_t)(overFlow&0x000000FF),
							  (uint8_t)((overFlow&0x0000FF00)>>8),
							  (uint8_t)((overFlow&0x00FF0000)>>16));
						while(isTimer0Done()==0) if(checkKeySkip()) return 0;
											
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
}//end function