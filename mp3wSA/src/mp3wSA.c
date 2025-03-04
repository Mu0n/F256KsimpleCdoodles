#define F256LIB_IMPLEMENTATION

#define CHUNK8K 0x2000
#define CHUNK4K 0x1000
#define CHUNK2K 0x0800
#define CHUNK1K 0x0400
#define CHUNK128B 0x80
#define CHUNK64B 0x40
#define CHUNK32B 0x20

#include "f256lib.h"
#include "../src/muUtils.h"
#include "../src/muVS1053b.h"

void read8KChunk(void *, FILE *);
uint8_t openMP3File(char *);
uint32_t totalsize = 10295603;

FILE *theMP3file;


uint8_t openMP3File(char *name)
{
	printf("Opening file: %s\n",name);
	theMP3file = fopen(name,"rb"); // open file in read mode
	if(theMP3file == NULL)
	{
		printf("Couldn't open the file: %s\n",name);
		return 1;
	}
	return 0;
}

void read8KChunk(void *buf, FILE *f)
{
	uint16_t i;
	for(i=0;i<64;i++)
		{
		fread((void *)(buf+i*0x80), sizeof(uint8_t), 128, f); //read 128 bytes at a time, since there's a hard limit of 255 reads at a time. 64x128 = 8k = 8192 bytes
		}
}

void backgroundSetup()
{
	uint16_t c=0;
	
	POKE(MMU_IO_CTRL, 0x00);
	// XXX GAMMA  SPRITE   TILE  | BITMAP  GRAPH  OVRLY  TEXT
	POKE(VKY_MSTR_CTRL_0, 0b00101111); //sprite,graph,overlay,text
	// XXX XXX  FON_SET FON_OVLY | MON_SLP DBL_Y  DBL_X  CLK_70
	POKE(VKY_MSTR_CTRL_1, 0b00000100); //font overlay, double height text, 320x240 at 60 Hz;
	POKE(VKY_LAYER_CTRL_0, 0b00000001); //bitmap 1 in layer 0, bitmap 0 in layer 1
	POKE(VKY_LAYER_CTRL_1, 0b00000010); //bitmap 2 in layer 2
	POKE(0xD00D,0x00); //force black graphics background
	POKE(0xD00E,0x00);
	POKE(0xD00F,0x00);

	bitmapSetActive(0);
	bitmapSetCLUT(0);
	
	bitmapSetVisible(0,false);
	bitmapSetVisible(1,false);
	bitmapSetVisible(2,false);
}
int main(int argc, char *argv[]) {
	
	uint16_t i=0,j=0;
	uint32_t fileIndex=0; //index to keep track of where we're at in the file
	uint16_t bufferIndex=0; //index for the local 8k buffer register
    uint16_t readEntryBufferIndex = 0;
	uint16_t rawFIFOCount=0;
	uint16_t bytesToTopOff=0;
	uint16_t multipleOf64b = 0;
	uint8_t visualX=0;

	char buffer[CHUNK8K]; //4x the size of the VS FIFO buffer

    backgroundSetup();
	openAllCODEC();
	boostVSClock();
	initSpectrum();
	
	openMP3File("ronald.mp3");
	//openMP3File("terranone.mp3");
	read8KChunk((void *)buffer, theMP3file); //read the first 8k chunk from the .mp3 file	
	fileIndex+=CHUNK8K; //advance the file index by 8kb
	
	printf("Hit space to start playback on a F256K2 or a F256Jr2");
	hitspace();
	
	printf("\nPlayback launched for 'Try the Bass' from Ronald Jenkees");

	for(i=bufferIndex;i<bufferIndex+CHUNK2K;i++) //fill the first 2k chunk into the full size of the buffer
		{
		POKE(VS_FIFO_DATA, buffer[i]);
		}
	bufferIndex+=CHUNK2K;
	for(i=bufferIndex;i<bufferIndex+CHUNK2K;i++) //fill the first 2k chunk into the full size of the buffer
		{
		POKE(VS_FIFO_DATA, buffer[i]);
		}
	bufferIndex+=CHUNK2K;
	
	
	printf("\nStreaming mp3 data to the VS1053b onboard chip.");
	printf("\n\nBuffer in 64b chunks:");
	while(fileIndex<totalsize)
	{
		rawFIFOCount = PEEKW(VS_FIFO_STAT);
		bytesToTopOff = CHUNK2K - (rawFIFOCount&0x0FFF); //found how many bytes are left in the 2KB buffer
		multipleOf64b = bytesToTopOff>>6; //multiples of 64 bytes of stuff to top off the FIFO buffer
		textGotoXY(0,7);textPrintInt(32-multipleOf64b);textPrint(" ");
		/* just a bit of visuals can make it choke in initial tests
		visualX=multipleOf64b<<2;
		bitmapSetColor(0xFF);bitmapLine(1,50,visualX,50);
		bitmapSetColor(0x3D);bitmapLine(visualX,50,128,50);*/
		for(i=0; i<multipleOf64b; i++)
		{
			for(j=0; j<CHUNK64B; j++)
			{
			POKE(VS_FIFO_DATA, buffer[bufferIndex+j]);
			}
			//advance the buffer index
			bufferIndex+=CHUNK64B;
			if(bufferIndex == CHUNK8K) bufferIndex = 0; //warp over if the 8KB limit is reached
			
			//read more of the file
			fread((void *)(buffer+readEntryBufferIndex), sizeof(uint8_t), 64, theMP3file); //read 64 bytes
			readEntryBufferIndex+=CHUNK64B; //keep track of where we're at so we can finish the file
			if(readEntryBufferIndex== CHUNK8K) readEntryBufferIndex = 0; //keep track of where we're at so we can finish the file
		}
	}
	
fclose(theMP3file);
printf("\nPlayback ended. Press space to quit");
hitspace();
return 0;}


