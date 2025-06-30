#include "D:\F256\llvm-mos\code\multiRead\.builddir\trampoline.h"

#define F256LIB_IMPLEMENTATION

#include "f256lib.h"
#include "../src/muUtils.h" //contains helper functions I often use
#include "../src/muMidi.h" //contains helper functions I often use

#define MIDISTART 0x10000

EMBED(music, "../Asayake.mid", 0x10000); //1kb

<<<<<<< Updated upstream
uint16_t readBigEndian16(FILE *fp) {
    uint8_t bytes[2];
    if (fread(bytes, 1, 2, fp) != 2) {
        // Handle error — possibly end of file
        return 0;
    }
=======
typedef struct MIDITrackParser
{
	uint32_t length, offset, start;
	uint32_t delta;
	uint8_t cmd[3];
	uint8_t lastCmd;
	bool is2B;
	bool isDone;
} MIDTrackP;

typedef struct MIDIParser
{
	uint16_t nbTracks;
	struct MIDITrackParser *tracks;
} MIDP;


void initTrack(MIDTrackP *mtp)
{
	mtp->length = 0;
	mtp->offset = 0;
	mtp->start = 0;
	mtp->delta = 0;
	mtp->cmd[0] = mtp->cmd[1] = mtp->cmd[2] = 0;
	mtp->lastCmd = 0;
	mtp->is2B = true;
	mtp->isDone = false;
}

uint16_t peekBigEndian16(uint32_t where) {
    uint8_t bytes[2];
	bytes[0] = FAR_PEEK(where);
	bytes[1] = FAR_PEEK(where+1);
	
>>>>>>> Stashed changes
    return ((uint16_t)bytes[0] << 8) |
           (uint16_t)bytes[1];
}


uint32_t peekBigEndian32(uint32_t where) {
    uint8_t bytes[4];
<<<<<<< Updated upstream
    if (fread(bytes, 1, 4, fp) != 4) {
        // Handle error — possibly end of file
        return 0;
    }
	printf("%02x %02x %02x %02x\n", bytes[0], bytes[1], bytes[2], bytes[3]);
=======
 
	bytes[0] = FAR_PEEK(where);
	bytes[1] = FAR_PEEK(where+1);
	bytes[2] = FAR_PEEK(where+2);
	bytes[3] = FAR_PEEK(where+3);
	
>>>>>>> Stashed changes
    return ((uint32_t)bytes[0] << 24) |
           ((uint32_t)bytes[1] << 16) |
           ((uint32_t)bytes[2] << 8)  |
           (uint32_t)bytes[3];
}

<<<<<<< Updated upstream

=======
//reads the time delta
uint32_t readDelta(MIDP *theOne, uint8_t track)
{
	uint32_t nValue, nValue2, nValue3, nValue4;
	uint8_t temp;
	
	nValue = 0x00000000;
	nValue2 = 0x00000000;
	nValue3 = 0x00000000;
	nValue4 = 0x00000000;
	
	temp = FAR_PEEK(theOne->tracks[track].start + theOne->tracks[track].offset++);
	nValue = (uint32_t)temp;
	
	if(nValue & 0x00000080)
		{
		nValue &= 0x0000007F;
		nValue <<= 7;
	
	
		temp = FAR_PEEK(theOne->tracks[track].start + theOne->tracks[track].offset++);
		nValue2 = (uint32_t)temp;
	
		if(nValue2 & 0x00000080)
			{
			nValue2 &= 0x0000007F;
			nValue2 <<= 7;
			nValue <<= 7;
	
			temp = FAR_PEEK(theOne->tracks[track].start + theOne->tracks[track].offset++);
			nValue3 = (uint32_t)temp;
	
			if(nValue3 & 0x00000080)
				{
				nValue3 &= 0x0000007F;
				nValue3 <<= 7;
				nValue2 <<= 7;
				nValue <<= 7;
	
				temp = FAR_PEEK(theOne->tracks[track].start + theOne->tracks[track].offset++);
				nValue4 = (uint32_t)temp;
				} //end of getting to nValue4
			} //end of getting to nValue3
		} //end of getting to nValue2
	return nValue | nValue2 | nValue3 | nValue4;
}

uint8_t skipWhenFFCmd(MIDP *theOne, uint8_t track, uint8_t meta_byte, uint8_t data_byte)
{

	if(meta_byte == MetaSequence || meta_byte == MetaChannelPrefix || meta_byte == MetaChangePort)
		{
		theOne->tracks[track].offset+=(uint32_t)3;
		}
	else if(meta_byte == MetaText || meta_byte == MetaCopyright || meta_byte == MetaTrackName || meta_byte == MetaInstrumentName
			 || meta_byte == MetaLyrics || meta_byte == MetaMarker || meta_byte == MetaCuePoint)
		{
		theOne->tracks[track].offset+=(uint32_t)2+(uint32_t)data_byte;
		}
	else if(meta_byte == MetaEndOfTrack)
		{
		theOne->tracks[track].offset+=(uint32_t)2;
		return 2;
		}
	else if(meta_byte == MetaSetTempo)
		{
		theOne->tracks[track].offset+=(uint32_t)5;
	
			/*
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
	
*/
		}
	else if(meta_byte == MetaSMPTEOffset)
		{
		theOne->tracks[track].offset+=(uint32_t)7;
		}
	else if(meta_byte == MetaTimeSignature)
		{

		theOne->tracks[track].offset+=(uint32_t)6;
		}
	else if(meta_byte == MetaKeySignature)
		{
		theOne->tracks[track].offset+=(uint32_t)4;
		}
	else if(meta_byte == MetaSequencerSpecific)
		{
		}
	return 1;
}

uint8_t readMIDICmd(MIDP *theOne, uint8_t track)
{
	uint8_t status_byte, extra_byte, extra_byte2; //temporary bytes that fetch data from the file
	
//status byte or MIDI message reading

//	printf("\nT%d start at: %08lx + %08lx", track, theOne->tracks[track].start, theOne->tracks[track].offset);
	
	status_byte = FAR_PEEK(theOne->tracks[track].start + theOne->tracks[track].offset);
	extra_byte = FAR_PEEK(theOne->tracks[track].start + theOne->tracks[track].offset + (uint32_t)1);
	extra_byte2 = FAR_PEEK(theOne->tracks[track].start + theOne->tracks[track].offset + (uint32_t)2);
	
	theOne->tracks[track].cmd[0] = status_byte;
	theOne->tracks[track].cmd[1] = extra_byte;
	theOne->tracks[track].cmd[2] = extra_byte2;
	
	
	printf("\n%08lx %02x %02x %02x <check command", theOne->tracks[track].delta,
												theOne->tracks[track].cmd[0],
												theOne->tracks[track].cmd[1],
												theOne->tracks[track].cmd[2]);
	
	theOne->tracks[track].offset++; //advance the offset by the first it needs
//first, check for run-on commands that don't repeat the status_byte
	if(status_byte < 0x80) //run-on detected!
		{
		extra_byte2 = extra_byte; //the 2nd parameter of the command was here
		extra_byte = status_byte; //the first parameter of the command was here
		status_byte = theOne->tracks[track].lastCmd; //fetch this from the recorded last command

		//printf(" ! %02x", status_byte);
		theOne->tracks[track].offset--; //since there was no command byte, back track by one
		}
//second, deal with MIDI meta-event commands that start with 0xFF
	if(status_byte == 0xFF)
		{
		return skipWhenFFCmd(theOne, track, extra_byte, extra_byte2);
		}
//Third, deal with regular MIDI commands
//MIDI commands with only 1 data byte
//Program change   0xC_
//Channel Pressure 0xD_
	else if(status_byte >= 0xC0 && status_byte <= 0xDF)
		{
		theOne->tracks[track].offset++; //complete the 2 byte advance in the offset (or 1 if run-on)
		theOne->tracks[track].is2B = true;
		theOne->tracks[track].lastCmd = status_byte; //preserve this in case a run-on command happens next
	
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
		theOne->tracks[track].offset+=2; //complete the 3 byte advance in the offset (or 2 if run-on)
		theOne->tracks[track].lastCmd = status_byte; //preserve this in case a run-on command happens next
		theOne->tracks[track].is2B = false;
		return 0;
		}
	else
		{
		//printf("\n ---Unrecognized event sb= %02x",status_byte);
		}
		return 0;
}


//reads one midi command, returns how many bytes are needed for the command inside the buffer.
//uint8_t readMIDIEvent(MIDP *theOne, uint8_t track, FILE *fp)
uint8_t readMIDIEvent(MIDP *theOne, uint8_t track)
{
	theOne->tracks[track].delta = readDelta(theOne, track);
	return readMIDICmd(theOne, track);
}
>>>>>>> Stashed changes
int main(int argc, char *argv[]) {
	uint8_t *fileNum =0, *fileNum2 = 0;
	uint8_t cmdBuf[8], cmdBuf2[8];
	uint16_t nbTracks = 0;
	uint32_t pos;
	
<<<<<<< Updated upstream
	uint32_t *lengths, *offsets, *starts;
	bool *isDone;
	//size_t binLen = sizeof(binData);
	
=======
>>>>>>> Stashed changes
	//setup and open file
	
	for(uint8_t i=0; i<8; i++) cmdBuf[i] = 0; // clear out
<<<<<<< Updated upstream
	
	fileNum  = fileOpen("Asayake.mid","r");
	fileNum2 = fileOpen("Asayake.mid","r");
	
	
	//read number of tracks
	fseek(fileNum, 10, SEEK_SET);
	nbTracks = readBigEndian16(fileNum);  // Read number of tracks
	
	lengths = (uint32_t *) malloc(sizeof(uint32_t) * nbTracks);
	offsets = (uint32_t *) malloc(sizeof(uint32_t) * nbTracks);
	starts = (uint32_t *) malloc(sizeof(uint32_t) * nbTracks);
	isDone = (bool *) malloc(sizeof(bool) * nbTracks);
=======

	
	//read number of tracks
	theOne.nbTracks = peekBigEndian16(MIDISTART+(uint32_t)10);

	theOne.tracks = (MIDTrackP *)malloc(sizeof(MIDTrackP) * theOne.nbTracks);
>>>>>>> Stashed changes
	
	for(uint16_t i = 0; i< nbTracks; i++)
	{
		isDone[i] = false;
		lengths[i] = 0;
		starts[i] = 0;
		offsets[i] = 0;
	}
	
	
	hitspace();
	//go to every track
<<<<<<< Updated upstream
	pos=14;
	fseek(fileNum, pos, SEEK_SET);  // Skip header chunk
	
	for (uint16_t i = 0; i < nbTracks; i++) {
	
		fread(cmdBuf, 1, 4, fileNum);//read track signature
=======
	
    pos=14;
	
	//find the start positions of every track
	for (uint16_t i = 0; i < theOne.nbTracks; i++) {
		cmdBuf[0] = FAR_PEEK(MIDISTART + pos);
		cmdBuf[1] = FAR_PEEK(MIDISTART + pos + (uint32_t)1);
		cmdBuf[2] = FAR_PEEK(MIDISTART + pos + (uint32_t)2);
		cmdBuf[3] = FAR_PEEK(MIDISTART + pos + (uint32_t)3);
		pos+=4;
>>>>>>> Stashed changes
	
		if (strcmp(cmdBuf, "MTrk") != 0) {
			// Handle error: unexpected chunk
		}
	
<<<<<<< Updated upstream
		uint32_t length = readBigEndian32(fileNum); // read track byte length
		lengths[i] = length;
		pos+=(uint32_t)8;
		printf("Track %d starts at offset %08lx\n", i, pos);
		starts[i] = pos; //know where to begin
	
		pos+=length;
		fseek(fileNum, pos, SEEK_SET);  // Skip to next chunk
=======
		uint32_t length  = peekBigEndian32(MIDISTART + pos); // read track byte length
		theOne.tracks[i].length = length;
		pos+=4;
		printf("Track %d starts at offset %08lx\n", i, pos);
		theOne.tracks[i].start = MIDISTART + pos; //know where to begin
	
		pos+=length;
>>>>>>> Stashed changes
	}

<<<<<<< Updated upstream
	while(true) {
	for(uint16_t i = 0; i < nbTracks; i++)
		{
		if(lengths[i]==0) continue;
		fseek(fileNum, starts[i] + offsets[i], SEEK_SET);
		fileRead(cmdBuf, 1,1, fileNum);
		textGotoXY(0 + offsets[i]*3,15 + i*2); printf("%02x ", cmdBuf[0]);
		offsets[i]++;
=======
//first pass, just get one event per track, renew if it's non important event
for(uint16_t i = 0; i < theOne.nbTracks; i++)
	{
		bool quitRefresh = false;
		printf("\nT%d ", i);
		if(theOne.tracks[i].isDone) continue; //skip finished tracks
		if(theOne.tracks[i].offset >= theOne.tracks[i].length)
		{
			theOne.tracks[i].isDone = true; //mark it as finished if we reach the end
			continue;
		}
		for(;;)
		{
			switch(readMIDIEvent(&theOne, i))
			{
				case 1:
					continue;
				case 0:
					quitRefresh = true;
					break;
				case 2:
					theOne.tracks[i].isDone = true;
					quitRefresh = true;
					break;
			}
			if(quitRefresh) break;
		}
	}
	

while(true)
{
bool quitRefresh = false;
uint32_t lowest = 0xFFFFFFFF;
uint8_t lowestIndex = 0xFF;

for(uint16_t i = 0; i < theOne.nbTracks; i++)
	{
		if(theOne.tracks[i].delta == 0)
		{
			lowest = 0;
			lowestIndex = i;
			break;
		}
		if(theOne.tracks[i].delta < lowest)
		{
			lowest = theOne.tracks[i].delta;
			lowestIndex = i;
		}
	}

POKE(MIDI_FIFO, theOne.tracks[lowestIndex].cmd[0]);
POKE(MIDI_FIFO, theOne.tracks[lowestIndex].cmd[1]);

if(!theOne.tracks[lowestIndex].is2B)
{
POKE(MIDI_FIFO, theOne.tracks[lowestIndex].cmd[2]);
}

//correct others
for(uint16_t i = 0; i < theOne.nbTracks; i++)
	{
		if(theOne.tracks[lowestIndex].delta == 0) break;
		if(i==lowestIndex) continue;
		theOne.tracks[i].delta -= theOne.tracks[lowestIndex].delta;
	}

//renew the one spent
if(theOne.tracks[lowestIndex].isDone == false && theOne.tracks[lowestIndex].offset < theOne.tracks[lowestIndex].length)
	{
	for(;;)
		{
			switch(readMIDIEvent(&theOne, lowestIndex))
			{
				case 1:
					continue;
				case 0:
					quitRefresh = true;
					break;
				case 2:
					theOne.tracks[lowestIndex].isDone = true;
					quitRefresh = true;
					break;
			}
			if(quitRefresh) break;
>>>>>>> Stashed changes
		}
	}
	
<<<<<<< Updated upstream

	printf("file Num is: %d\n", *fileNum);
	
	
	
	fileClose(fileNum);
	fileClose(fileNum2);
=======
}





>>>>>>> Stashed changes
	
	printf("hit space to quit");
	hitspace();
	
	return 0;}
