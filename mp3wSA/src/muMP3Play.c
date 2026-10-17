
#include "f256lib.h"
#include "../src/muMP3Play.h"
#include "../src/muVS1053b.h"
#include "../src/muFileDiplomacy.h"


//globals
static FILE *theMP3File;

bool nearingEnd = false;
bool isMP3Finished = true;

uint8_t prepMP3ForPlay(char *name) { 
	theMP3File = fileOpen(name,"rb"); // open file in read mode
	if(theMP3File == NULL) return 1; //something wrong
	initialInjectMP3();

	return 0;
}

bool isMP3Done() {
	return isMP3Finished;
}
void initialInjectMP3()
{
	read2KChunk();
	isMP3Finished = false;
}

void rewindAndPlayMP3() {
	initialInjectMP3();
}
void closeDownMp3File()
{
if(theMP3File != NULL) fileClose(theMP3File);
theMP3File = NULL;
}

void read2KChunk() {
	uint16_t i,j;
	uint8_t buf32[64];
	uint8_t mp3Buffer[CHUNK2K];
	
	for(i=0;i<32;i++)
		{
		fileRead(buf32, sizeof(uint8_t), 64, theMP3File); //read 64 bytes at a time, since there's a hard limit of 255 reads at a time. 64x128 = 8k = 8192 bytes
		for(j=0;j<64;j++)
			{
			mp3Buffer[j + i*64] = buf32[j];
			}
		}
	for(uint16_t k=0;k<CHUNK2K;k++) //fill the first 2k chunk into the full size of the buffer
		{
		POKE(VS_FIFO_DATA, mp3Buffer[k]);
		}
}

int8_t MP3LoopPass()
{
	uint8_t i, j;
	uint16_t rawFIFOCount=0;
	uint16_t bytesToTopOff=0;
	uint16_t multipleOf32b = 0;
	uint8_t deliveredBytes=0;
	uint8_t buf32[32];
	
	//if(globalPause) return 0; //globalPause is from muTimerInt0.h
	if(isMP3Finished) return 0;
	
	//Check the health of the FIFO buffer
	rawFIFOCount = PEEKW(VS_FIFO_COUNT);
	bytesToTopOff = CHUNK2K - (rawFIFOCount&0x07FF); //found how many bytes are left in the 2KB buffer
	multipleOf32b = bytesToTopOff>>7; //multiples of 32 bytes of stuff to top off the FIFO buffer

	for(i=0; i<multipleOf32b; i++)
		{			
		deliveredBytes = fileReadDiplomat(buf32, sizeof(uint8_t), CHUNK32B, theMP3File);
		if(deliveredBytes !=CHUNK32B) isMP3Finished = true; //Check if we're getting to the end of the file
		for(j=0; j<deliveredBytes; j++) 
			{
			POKE(VS_FIFO_DATA, buf32[j]);
			}

		if(isMP3Finished)
			{
			silencePad();
			closeDownMp3File();
			return 1;
			}
		}
	return 0;
}

void silencePad()
{
	for(uint8_t i=0;i<32;i++) //do this 32 times * 64 bytes = 2048 bytes
	{
	while(2048 - (PEEKW(VS_FIFO_COUNT)&0x07FF) < 64)
		; //wait for a 64 bytes space that's free in the FIFO at least
		
	for(uint8_t j=0;j<64;j++)	POKE(VS_FIFO_DATA, SILENTBYTE);	//empty out the next 64 bytes
	}
}
