#define F256LIB_IMPLEMENTATION

#include "f256lib.h"
#include "../src/muUtils.h" //contains helper functions I often use
#include "../src/muMidi.h" //contains helper functions I often use
#include "../src/muTimer0Int.h" //contains helper functions I often use

#define MIDISTART 0x10000

EMBED(music, "../Asayake.mid", 0x10000); //1kb

typedef struct MIDITrackParser {
	uint32_t length, offset, start;
	uint32_t delta;
	uint8_t cmd[3];
	uint8_t lastCmd;
	bool is2B;
	bool isDone;
} MIDTrackP;

typedef struct MIDIParser {
	uint16_t nbTracks;
	uint16_t ticks;
	bool isWaiting;
	uint32_t cuedDelta;
	uint16_t cuedIndex;
	struct MIDITrackParser *tracks;
} MIDP;


struct MIDIParser theOne;

void initTrack(void);
void playMidi(void);
uint8_t readMIDIEvent(uint8_t);
uint16_t readBigEndian16(uint32_t);
uint32_t readBigEndian32(uint32_t);
uint32_t timer0PerTick;




void initTrack(){
	for(uint16_t i=0; i<theOne.nbTracks; i++)
	{
	theOne.tracks[i].length = 0;
	theOne.tracks[i].offset = 0;
	theOne.tracks[i].start = 0;
	theOne.tracks[i].delta = 0;
	theOne.tracks[i].cmd[0] = theOne.tracks[i].cmd[1] = theOne.tracks[i].cmd[2] = 0;
	theOne.tracks[i].lastCmd = 0;
	theOne.tracks[i].is2B = true;
	theOne.tracks[i].isDone = false;
	}
}

uint16_t readBigEndian16(uint32_t where) {
    uint8_t bytes[2];
	bytes[0] = FAR_PEEK(where);
	bytes[1] = FAR_PEEK(where+1);
	
    return ((uint16_t)bytes[0] << 8) |
           (uint16_t)bytes[1];
}


uint32_t readBigEndian32(uint32_t where) {
    uint8_t bytes[4];

	bytes[0] = FAR_PEEK(where);
	bytes[1] = FAR_PEEK(where+1);
	bytes[2] = FAR_PEEK(where+2);
	bytes[3] = FAR_PEEK(where+3);
	
    return (((uint32_t)bytes[0]) << 24) |
           (((uint32_t)bytes[1]) << 16) |
           (((uint32_t)bytes[2]) << 8)  |
           (uint32_t)bytes[3];
}


//reads the time delta
uint32_t readDelta(uint8_t track) {
	uint32_t nValue, nValue2, nValue3, nValue4;
	uint8_t temp;
	
	nValue = 0x00000000;
	nValue2 = 0x00000000;
	nValue3 = 0x00000000;
	nValue4 = 0x00000000;
	
	temp = FAR_PEEK(theOne.tracks[track].start + theOne.tracks[track].offset++);
	nValue = (uint32_t)temp;
	
	if(nValue & 0x00000080)
		{
		nValue &= 0x0000007F;
		nValue <<= 7;
		
		
		temp = FAR_PEEK(theOne.tracks[track].start + theOne.tracks[track].offset++);
		nValue2 = (uint32_t)temp;
		
		if(nValue2 & 0x00000080)
			{
			nValue2 &= 0x0000007F;
			nValue2 <<= 7;
			nValue <<= 7;
			
			temp = FAR_PEEK(theOne.tracks[track].start + theOne.tracks[track].offset++);
			nValue3 = (uint32_t)temp;
			
			if(nValue3 & 0x00000080)
				{
				nValue3 &= 0x0000007F;
				nValue3 <<= 7;
				nValue2 <<= 7;
				nValue <<= 7;
				
				temp = FAR_PEEK(theOne.tracks[track].start + theOne.tracks[track].offset++);
				nValue4 = (uint32_t)temp;
				} //end of getting to nValue4
			} //end of getting to nValue3
		} //end of getting to nValue2	
	return nValue | nValue2 | nValue3 | nValue4;
}

uint8_t skipWhenFFCmd(uint8_t track, uint8_t meta_byte, uint8_t data_byte) {	
	if(meta_byte == MetaSequence || meta_byte == MetaChannelPrefix || meta_byte == MetaChangePort)
		{
		theOne.tracks[track].offset+=(uint32_t)3;
		}
	else if(meta_byte == MetaText || meta_byte == MetaCopyright || meta_byte == MetaTrackName || meta_byte == MetaInstrumentName
			 || meta_byte == MetaLyrics || meta_byte == MetaMarker || meta_byte == MetaCuePoint)
		{
		theOne.tracks[track].offset+=(uint32_t)2+(uint32_t)data_byte;
		}
	else if(meta_byte == MetaEndOfTrack)
		{
		theOne.tracks[track].offset+=(uint32_t)2;
		return 2;
		}
	else if(meta_byte == MetaSetTempo)
		{
		theOne.tracks[track].offset+=(uint32_t)1;
			
		uint8_t data_byte2 = FAR_PEEK(theOne.tracks[track].start+theOne.tracks[track].offset+(uint32_t)1);
		uint8_t data_byte3 = FAR_PEEK(theOne.tracks[track].start+theOne.tracks[track].offset+(uint32_t)2);
		uint8_t data_byte4 = FAR_PEEK(theOne.tracks[track].start+theOne.tracks[track].offset+(uint32_t)3);
		
		theOne.tracks[track].offset+=(uint32_t)4;
		
		uint32_t usPerBeat = ( ((uint32_t)data_byte2)<<16 ) | 
					   ( ((uint32_t)data_byte3)<<8  ) | 
						 ((uint32_t)data_byte4);
						 
		
		//if you divide usPerBeat by tick per beat, 
		//you get the duration in microseconds per tick, ready to be multiplied	by the events' deltaTimes to get delays in us

		uint32_t usPerTick = (uint32_t)usPerBeat/((uint32_t)theOne.ticks);
		
		timer0PerTick = (uint32_t)((double)usPerTick * (double)25.0f); //convert to the units of timer0
		textGotoXY(20,11);printf("%08lx deltaTransfor",timer0PerTick);
		theOne.tracks[track].delta = timer0PerTick;

		}
	else if(meta_byte == MetaSMPTEOffset)
		{
		theOne.tracks[track].offset+=(uint32_t)7;
		}
	else if(meta_byte == MetaTimeSignature)
		{

		theOne.tracks[track].offset+=(uint32_t)6;
		}
	else if(meta_byte == MetaKeySignature)
		{
		theOne.tracks[track].offset+=(uint32_t)4;
		}
	else if(meta_byte == MetaSequencerSpecific)
		{
		}
	return 1;
}

uint8_t readMIDICmd(uint8_t track) {
	uint8_t status_byte, extra_byte, extra_byte2; //temporary bytes that fetch data from the file
	
//status byte or MIDI message reading

//	printf("\nT%d start at: %08lx + %08lx", track, theOne.tracks[track].start, theOne.tracks[track].offset); 
	
	status_byte = FAR_PEEK(theOne.tracks[track].start + theOne.tracks[track].offset);
	extra_byte = FAR_PEEK(theOne.tracks[track].start + theOne.tracks[track].offset + (uint32_t)1);
	extra_byte2 = FAR_PEEK(theOne.tracks[track].start + theOne.tracks[track].offset + (uint32_t)2);
	
	theOne.tracks[track].cmd[0] = status_byte;
	theOne.tracks[track].cmd[1] = extra_byte;
	theOne.tracks[track].cmd[2] = extra_byte2;
	
	theOne.tracks[track].offset++; //advance the offset by the first it needs
//first, check for run-on commands that don't repeat the status_byte
	if(status_byte < 0x80) //run-on detected!
		{
		extra_byte2 = extra_byte; //the 2nd parameter of the command was here
		extra_byte = status_byte; //the first parameter of the command was here
		status_byte = theOne.tracks[track].lastCmd; //fetch this from the recorded last command


		theOne.tracks[track].cmd[0] = status_byte;
		theOne.tracks[track].cmd[0] = status_byte;
		theOne.tracks[track].cmd[1] = extra_byte;
		theOne.tracks[track].cmd[2] = extra_byte2;
		
		//printf(" ! %02x", status_byte);
		theOne.tracks[track].offset--; //since there was no command byte, back track by one	
		}
//second, deal with MIDI meta-event commands that start with 0xFF
	if(status_byte == 0xFF)
		{
		return skipWhenFFCmd(track, extra_byte, extra_byte2);
		}
//Third, deal with regular MIDI commands			
//MIDI commands with only 1 data byte
//MIDI commands with only 1 data byte
//Program change   0xC_
//Channel Pressure 0xD_		
	else if(status_byte >= 0xC0 && status_byte <= 0xDF)
		{
		theOne.tracks[track].offset++; //complete the 2 byte advance in the offset (or 1 if run-on)
		theOne.tracks[track].is2B = true;
		theOne.tracks[track].lastCmd = status_byte; //preserve this in case a run-on command happens next
		
		return 0;
		}
				
//MIDI commands with 2 data bytes
// Note off 0x8_
// Note on  0x9_
// Polyphonic Key Pressure 0xA_ (aftertouch)
// Control Change 0xB_
// (0xC_ and 0xD_ have been taken care of above already)
// Pitch Bend 0xE_
	else if((status_byte >= 0x80 && status_byte <= 0xBF) || (status_byte >= 0xE0 && status_byte <= 0xEF))
		{
			
		theOne.tracks[track].lastCmd = status_byte; //preserve this in case a run-on command happens next
		
		if((status_byte & 0xF0) == 0x90 && extra_byte2==0x00) 
			{
			status_byte = status_byte & 0x8F;	//sometimes note offs are note ons with 0 velocity, quirk of some midi sequencers
			extra_byte2 = 0x7F;
			}
					
					
		theOne.tracks[track].offset+=2; //complete the 3 byte advance in the offset (or 2 if run-on)		
		theOne.tracks[track].is2B = false;
		return 0;
		}
	else
		{
		//printf("\n ---Unrecognized event sb= %02x",status_byte);
		}
		return 0;
}

//get the next event in this track. do check out if it is "isDone" first, then get into this if it is!
void chainEvent(uint8_t track)
{
	bool quitRefresh = false; //keep looking until we get a case 0, important MIDI event we shouldn't skip, or a case 2 =end of track
	for(;;)
		{
		switch(readMIDIEvent(track))
			{
			case 1: //a skippable 0xFF event was detected, go to the next
				continue;
			case 0: //a regular event was detected, was not skipped
				quitRefresh = true;
				break;
			case 2: //a 0xFF 0x2F event was detected, end of track
				theOne.tracks[track].isDone = true;
				quitRefresh = true;
				break;
			}
		if(quitRefresh) break;
		}
}

void performMIDICmd(uint8_t track)
{
	POKE(MIDI_FIFO, theOne.tracks[track].cmd[0]);
	POKE(MIDI_FIFO, theOne.tracks[track].cmd[1]);

	if(theOne.tracks[track].is2B==false)
		{
		POKE(MIDI_FIFO, theOne.tracks[track].cmd[2]);
		}
}

//reads one midi command, returns how many bytes are needed for the command inside the buffer.
//uint8_t readMIDIEvent(MIDP *theOne, uint8_t track, FILE *fp)
uint8_t readMIDIEvent(uint8_t track) {
	theOne.tracks[track].delta = readDelta(track) * timer0PerTick;
	return readMIDICmd(track);
}

void exhaustZeroes(uint8_t track)
{
	for(;;)
	{	
		performMIDICmd(track);
				
		chainEvent(track);
		if(theOne.tracks[track].isDone) return;
		if(theOne.tracks[track].delta > 0) return;
	}
}

/**
 * Handle interrupts
 */
__attribute__((noinline)) __attribute__((interrupt_norecurse))
void irq_handler() {
    byte irq0 = PEEK(INT_PENDING_0);
    if ((irq0 & INT_TIMER_0) > 0) {
		byte LUT = PEEK(0);
		POKE(INT_PENDING_0, irq0 & 0xFE);
		POKE(0,0xB3);
		
	printf("...");
		playMidi();
		POKE(0, LUT);
    }
    // Handle other interrupts as normal
    //original_irq_handler();
    //asm volatile("jmp (%[mem])" : : [mem] "r" (original_irq_handler));
}

void playMidi()
{
//play stuff
	//printf("%08lx coucou",theOne.cuedDelta);
	if(theOne.cuedDelta > 0x00FFFFFF) //0x00FFFFFF is the max value of the timer0 we can do
		{
			//delay up to maximum of 0x00FFFFFF = 2/3rds of a second
		theOne.cuedDelta -= 0x00FFFFFF; //reduce the max value one by one until there is a remainder smaller than the max amount
		
		loadTimer(theOne.cuedDelta);
		return;
		}
	//do the last delay that's under 2/3rds of a second
	if(theOne.cuedDelta > 0)	
		{
		loadTimer(theOne.cuedDelta);
		return;		
		}
	performMIDICmd(theOne.cuedIndex);

	//
	//perform this after an event with a non-zero delay has been played, lower the other tracks' deltas by that amount, and refresh next event
	//
	for(uint16_t i = 0; i < theOne.nbTracks; i++)
		{
		if(theOne.tracks[i].isDone) continue; //this track is done
		if(i==theOne.cuedIndex) continue; //don't modify itself
		theOne.tracks[i].delta -= theOne.tracks[theOne.cuedIndex].delta;
		}

	//renew the one spent
	chainEvent(theOne.cuedIndex);
	if(theOne.tracks[theOne.cuedIndex].delta == 0) exhaustZeroes(theOne.cuedIndex);
	theOne.isWaiting = false;	
}
int main(int argc, char *argv[]) {
	uint8_t cmdBuf[8];
	uint32_t pos;	

	for(uint8_t i=0; i<8; i++) cmdBuf[i] = 0; // clear out

	//read number of tracks
	theOne.nbTracks = readBigEndian16(MIDISTART+(uint32_t)10);
	theOne.tracks = (MIDTrackP *)malloc(sizeof(MIDTrackP) * theOne.nbTracks);
	theOne.isWaiting = false;
	theOne.cuedDelta = 0xFFFFFFFF;
	theOne.cuedIndex = 0;
	
	printf("nbTracks %d \n", theOne.nbTracks);
	for(uint16_t j = 0; j< theOne.nbTracks; j++) //finish preparing the main structure
		{
		initTrack();
		}
		
	//read tick
	theOne.ticks = readBigEndian16(MIDISTART+(uint32_t)12);
	//go to every track
	pos=14;
	
	//find the start positions of every track
	for (uint16_t i = 0; i < theOne.nbTracks; i++) {
		cmdBuf[0] = FAR_PEEK(MIDISTART + pos);
		cmdBuf[1] = FAR_PEEK(MIDISTART + pos + (uint32_t)1);
		cmdBuf[2] = FAR_PEEK(MIDISTART + pos + (uint32_t)2);
		cmdBuf[3] = FAR_PEEK(MIDISTART + pos + (uint32_t)3);
		pos+=4;
		
		if (strcmp(cmdBuf, "MTrk") != 0) {
			// Handle error: unexpected chunk
			}

		uint32_t length  = readBigEndian32(MIDISTART + pos); // read track byte length
		theOne.tracks[i].length = length;
		pos+=4;
		printf("Track %d starts at offset %08lx and is length %08lx\n", i, pos, length);
		theOne.tracks[i].start = MIDISTART + pos; //know where to begin
		pos +=length;
		}
		

//first pass, just get one event per track, renew if it's non important event

	for(uint16_t i = 0; i < theOne.nbTracks; i++)
		{
		if(theOne.tracks[i].isDone) continue; //skip finished track, go to next
		if(theOne.tracks[i].offset >= theOne.tracks[i].length) //it's already done, go to next
			{
			theOne.tracks[i].isDone = true; //mark it as finished if we reach the end
			continue;
			}
		textGotoXY(0, 15 + i);
		
		chainEvent(i);
		}		
	
//find what to do and exhaust all zeroes
	for(uint16_t i = 0; i < theOne.nbTracks; i++)
		{
		if(theOne.tracks[i].isDone == false)
			{	
			if(theOne.tracks[i].delta == 0x00000000) exhaustZeroes(i);
			}
		}

		
//kick off the timer
	asm("SEI");
	enableTimer(irq_handler, playMidi);
	resetTimer0();
	asm("CLI");
	setTimer0(0x0000DEAD);
	
			printf(".");
	//insert game loop here		
	while(true)
		{
//find what to do and cue up the lowest non-zero	
		if(theOne.isWaiting == false)
			{
			for(uint16_t i = 0; i < theOne.nbTracks; i++)
				{
				uint32_t lowest = 0xFFFFFFFF;
				uint32_t lowestIndex = 0xFFFF;
				if(theOne.tracks[i].isDone == false && theOne.tracks[i].delta < lowest) //otherwise find the event planned for execution the soonest
					{
					lowest = theOne.tracks[i].delta;
					lowestIndex = i;
					theOne.isWaiting = true;
					}
				theOne.cuedDelta = lowest;
				theOne.cuedIndex = lowestIndex;
				if(lowest > 0x00FFFFFF) loadTimer(0x00FFFFFF);
				else loadTimer(theOne.tracks[lowestIndex].delta);
				}
			}
		}

	}	
}