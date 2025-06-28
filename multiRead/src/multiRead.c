#define F256LIB_IMPLEMENTATION

#include "f256lib.h"
#include "../src/muUtils.h" //contains helper functions I often use

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

uint16_t readBigEndian16(FILE *fp) {
    uint8_t bytes[2];
    if (fileRead(bytes, 1, 2, fp) != 2) {
        // Handle error — possibly end of file
		printf("error\n");
        return 0;
    }
    return ((uint16_t)bytes[0] << 8) |
           (uint16_t)bytes[1];
}



uint32_t readBigEndian32(FILE *fp) {
    uint8_t bytes[4];
    if (fileRead(bytes, 1, 4, fp) != 4) {
        // Handle error — possibly end of file
        return 0;
    }
	printf("%02x %02x %02x %02x\n", bytes[0], bytes[1], bytes[2], bytes[3]);
    return ((uint32_t)bytes[0] << 24) |
           ((uint32_t)bytes[1] << 16) |
           ((uint32_t)bytes[2] << 8)  |
           (uint32_t)bytes[3];
}

//reads the time delta
uint32_t readDelta(MIDP *theOne, uint8_t track, FILE *fileNum)
{
	uint32_t nValue, nValue2, nValue3, nValue4;
	uint8_t temp;
	
	nValue = 0x00000000;
	nValue2 = 0x00000000;
	nValue3 = 0x00000000;
	nValue4 = 0x00000000;
	
	
	fileSeek(fileNum, theOne->tracks[track].start + theOne->tracks[track].offset++, SEEK_SET);
	fread(&temp, 1, 1, fileNum);//read track signature
	nValue = (uint32_t)temp;
	
	if(nValue & 0x00000080)
		{
		nValue &= 0x0000007F;
		nValue <<= 7;
		
		fread(&temp, 1, 1, fileNum);//read track signature
		theOne->tracks[track].offset++;
		nValue2 = (uint32_t)temp;
		
		if(nValue2 & 0x00000080)
			{
			nValue2 &= 0x0000007F;
			nValue2 <<= 7;
			nValue <<= 7;
			
			fread(&temp, 1, 1, fileNum);//read track signature
			theOne->tracks[track].offset++;
			nValue3 = (uint32_t)temp;
			
			if(nValue3 & 0x00000080)
				{
				nValue3 &= 0x0000007F;
				nValue3 <<= 7;
				nValue2 <<= 7;
				nValue <<= 7;
				
				fread(&temp, 1, 1, fileNum);//read track signature
				theOne->tracks[track].offset++;
				nValue4 = (uint32_t)temp;
				} //end of getting to nValue4
			} //end of getting to nValue3
		} //end of getting to nValue2	
	return nValue | nValue2 | nValue3 | nValue4;
}

void skipWhenFFCmd(MIDP *theOne, uint8_t track, FILE *fp)
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
}

void readMIDICmd(MIDP *theOne, uint8_t track, FILE *fp)
{
	uint8_t status_byte, extra_byte, extra_byte2; //temporary bytes that fetch data from the file
	
//status byte or MIDI message reading
	fileSeek(fileNum, theOne->tracks[track].start + theOne->tracks[track].offset, SEEK_SET);
	fread(&status_byte, 1, 1, fileNum);
	fread(&extra_byte, 1, 1, fileNum); //be ready for 2 bytes midi events
	fread(&extra_byte2, 1, 1, fileNum); //be ready for 3 bytes midi events
	
	
	theOne->tracks[track].offset+=2; //advance the offset by at least 2, the minimum possible amount
//first, check for run-on commands that don't repeat the status_byte
	if(status_byte < 0x80)
		{
		extra_byte2 = extra_byte; //the 2nd parameter of the command was here
		extra_byte = status_byte; //the first parameter of the command was here
		status_byte = theOne->tracks[track].lastCmd; //fetch this from the recorded last command								
		}
//second, deal with MIDI meta-event commands that start with 0xFF
	if(status_byte == 0xFF)
		{
		skipWhenFFCmd(theOne, track, fp);
		}
//Third, deal with regular MIDI commands			
//MIDI commands with only 1 data byte
//Program change   0xC_
//Channel Pressure 0xD_		
	else if(status_byte >= 0xC0 && status_byte <= 0xDF)
		{
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
			
		}
	else
		{
		//printf("\n ---Unrecognized event sb= %02x",status_byte);
		}
	theOne->tracks[track].lastCmd = status_byte; //preserve this in case a run-on command happens next
}


//reads one midi command, returns how many bytes are needed for the command inside the buffer.
uint8_t readMIDIEvent(MIDP *theOne, uint8_t track, FILE *fp)
{
	theOne->tracks[track].delta = readDelta(theOne, track, fp);
	readMIDICmd(theOne, track, fp);
	
}
int main(int argc, char *argv[]) {
	uint8_t *fileNum =0;
	uint8_t cmdBuf[8];
	uint32_t pos;
	struct MIDIParser theOne;
	
	//size_t binLen = sizeof(binData);
	
	//setup and open file
	
	POKE(MMU_IO_CTRL, 0x00);
	// XXX GAMMA  SPRITE   TILE  | BITMAP  GRAPH  OVRLY  TEXT
	POKE(VKY_MSTR_CTRL_0, 0b00001111); //sprite,graph,overlay,text
	// XXX XXX  FON_SET FON_OVLY | MON_SLP DBL_Y  DBL_X  CLK_70
	POKE(VKY_MSTR_CTRL_1, 0b00010000); //font overlay, double height text, 320x240 at 60 Hz;
	
	for(uint8_t i=0; i<8; i++) cmdBuf[i] = 0; // clear out
	
	fileNum  = fileOpen("Asayake.mid","r");
	
	//read number of tracks
	fileSeek(fileNum, 10, SEEK_SET);
	theOne.nbTracks  = readBigEndian16(fileNum);  // Read number of tracks
	
	theOne.tracks = (MIDTrackP *)malloc(sizeof(MIDTrackP) * theOne.nbTracks);
	
	for(uint16_t i = 0; i< theOne.nbTracks; i++) initTrack(&theOne.tracks[i]); // initialize tracks
	
	//go to every track
	pos=14;
	fileSeek(fileNum, pos, SEEK_SET);  // Skip header chunk
	
	for (uint16_t i = 0; i < theOne.nbTracks; i++) {
		
		fread(cmdBuf, 1, 4, fileNum);//read track signature
		
		if (strcmp(cmdBuf, "MTrk") != 0) {
			// Handle error: unexpected chunk
		}
		
		uint32_t length = readBigEndian32(fileNum); // read track byte length
		theOne.tracks[i].length = length;
		pos+=(uint32_t)8;
		printf("Track %d starts at offset %08lx\n", i, pos);
		theOne.tracks[i].start = pos; //know where to begin
		
		pos+=length;
		fileSeek(fileNum, pos, SEEK_SET);  // Skip to next chunk
	}
	

	while(true) {
	for(uint16_t i = 0; i < theOne.nbTracks; i++)
		{
		if(theOne.tracks[i].isDone) continue; //skip finished tracks
		if(theOne.tracks[i].offset >= theOne.tracks[i].length) 
		{
			theOne.tracks[i].isDone = true; //mark it as finished if we reach the end
			continue;
		}
		readMIDIEvent(&theOne, i, fileNum);
		
		textGotoXY(0 + theOne.tracks[i].offset*3,15 + i*2); printf("%02x ", cmdBuf[0]);
		theOne.tracks[i].offset++;
		}
	hitspace();
	}
	
	
	fileClose(fileNum);
	
	printf("hit space to quit");
	hitspace();
	
	return 0;}
