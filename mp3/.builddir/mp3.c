#include "D:\F256\llvm-mos\code\mp3\.builddir\trampoline.h"

#define F256LIB_IMPLEMENTATION

#define VS_SCI_CTRL   0xD700
#define    CTRL_Start   0x01
#define    CTRL_RWn     0x02
#define    CTRL_Busy    0x08

#define VS_SCI_ADDR  0xD701
#define VS_SCI_DATA  0xD702   //2 bytes
#define VS_FIFO_STAT 0xD704   //2 bytes
#define VS_FIFO_DATA 0xD707

#define CHUNK8K 0x2000
#define CHUNK4K 0x1000
#define CHUNK2K 0x0800
#define CHUNK1K 0x0400
#define CHUNK128B 0x80
#define CHUNK64B 0x40
#define CHUNK32B 0x20

#include "f256lib.h"
#include "../src/muUtils.h"
#include "../src/muMidi.h"

EMBED(backg, "../assets/rj.raw.pal", 0x10000);
EMBED(palback, "../assets/rj.raw.bin", 0x10400);

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
	
	POKE(MMU_IO_CTRL,1);  //MMU I/O to page 1
	//prep to copy over the palette to the CLUT
	for(c=0;c<1023;c++)
	{
		POKE(VKY_GR_CLUT_0+c, FAR_PEEK(0x10000+c));
	}
	POKE(MMU_IO_CTRL,0);
	
	
	bitmapSetActive(0);
	bitmapSetAddress(0,0x10400);
	bitmapSetCLUT(0);
	
	bitmapSetVisible(0,true);
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
	uint16_t multipleOf32b = 0;
	uint8_t visualX=0;

	char buffer[CHUNK8K]; //4x the size of the VS FIFO buffer

	openAllCODEC();
	boostVSClock();
	backgroundSetup();
	
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
		multipleOf32b = bytesToTopOff>>6; //multiples of 64 bytes of stuff to top off the FIFO buffer
		textGotoXY(0,7);textPrintInt(32-multipleOf32b);textPrint(" ");
		/*visualX=multipleOf32b<<2;
		bitmapSetColor(0xFF);bitmapLine(1,50,visualX,50);
		bitmapSetColor(0x3D);bitmapLine(visualX,50,128,50);*/
		for(i=0; i<multipleOf32b; i++)
		{
			for(j=0; j<CHUNK64B; j++)
			{
			POKE(VS_FIFO_DATA, buffer[bufferIndex+j]);
			}
			//advance the buffer index
			bufferIndex+=CHUNK64B;
			if(bufferIndex == CHUNK8K) bufferIndex = 0; //warp over if the 8KB limit is reached
	
			//read more of the file
			fread((void *)(buffer+readEntryBufferIndex), sizeof(uint8_t), 64, theMP3file); //read 32 bytes
			readEntryBufferIndex+=CHUNK64B; //keep track of where we're at so we can finish the file
			if(readEntryBufferIndex== CHUNK8K) readEntryBufferIndex = 0; //keep track of where we're at so we can finish the file
		}
	}
	
	/*
	while(fileIndex<totalsize)
	{
		rawFIFOCount = PEEKW(VS_FIFO_STAT);
		bytesToTopOff = CHUNK2K - (rawFIFOCount&0x0FFF); //found how many bytes are left in the 2KB buffer
		multipleOf32b = bytesToTopOff>>7; //multiples of 128 bytes of stuff to top off the FIFO buffer
		textGotoXY(0,7);textPrintInt(multipleOf32b);textPrint(" ");
		visualX=multipleOf32b<<3;
		bitmapSetColor(0xFF);bitmapLine(1,50,visualX,50);
		bitmapSetColor(0x3D);bitmapLine(visualX,50,128,50);
		for(i=0; i<multipleOf32b; i++)
		{
			for(j=0; j<CHUNK128B; j++)
			{
			POKE(VS_FIFO_DATA, buffer[bufferIndex+j]);
			}
			//advance the buffer index
			bufferIndex+=CHUNK128B;
			if(bufferIndex == CHUNK8K) bufferIndex = 0; //warp over if the 8KB limit is reached
	
			//read more of the file
			fread((void *)(buffer+readEntryBufferIndex), sizeof(uint8_t), 128, theMP3file); //read 32 bytes
			readEntryBufferIndex+=CHUNK128B; //keep track of where we're at so we can finish the file
			if(readEntryBufferIndex== CHUNK8K) readEntryBufferIndex = 0; //keep track of where we're at so we can finish the file
		}
	}
	
	*/
/*
	while(fileIndex<totalsize)
	{
		rawFIFOCount = PEEKW(VS_FIFO_STAT);
		bytesToTopOff = CHUNK2K - (rawFIFOCount&0x0FFF); //found how many bytes are left in the 2KB buffer
		multipleOf32b = bytesToTopOff>>5; //multiples of 64 bytes of stuff to top off the FIFO buffer
		textGotoXY(0,10);textPrintInt(multipleOf32b);textPrint(" /64 ");
		for(i=0; i<multipleOf32b; i++)
		{
			for(j=0; j<CHUNK32B; j++)
			{
			POKE(VS_FIFO_DATA, buffer[bufferIndex+j]);
			}
			//advance the buffer index
			bufferIndex+=CHUNK32B;
			if(bufferIndex == CHUNK8K) bufferIndex = 0; //warp over if the 8KB limit is reached
	
			//read more of the file
			fread((void *)(buffer+readEntryBufferIndex), sizeof(uint8_t), 32, theMP3file); //read 32 bytes
			readEntryBufferIndex+=CHUNK32B; //keep track of where we're at so we can finish the file
			if(readEntryBufferIndex== CHUNK8K) readEntryBufferIndex = 0; //keep track of where we're at so we can finish the file
		}
	}


*/
fclose(theMP3file);
printf("\nPlayback ended. Press space to quit");
hitspace();
return 0;}


