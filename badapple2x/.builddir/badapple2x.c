#include "D:\F256\llvm-mos\code\badapple2x\.builddir\trampoline.h"

#define F256LIB_IMPLEMENTATION
#include "f256lib.h"
#include "../src/muUtils.h"
#include "../src/muVS1053b.h"


#define CHUNK8K 0x2000
#define CHUNK4K 0x1000
#define CHUNK2K 0x0800
#define CHUNK1K 0x0400
#define CHUNK128B 0x80
#define CHUNK64B 0x40
#define CHUNK32B 0x20


#define SET_PIXEL 7
#define CLEAR_PIXEL 32
#define TOPLEFT 0xC000
#define BUFFERSIZE 200

struct timer_t delay;

void read8KChunk(void *buf, FILE *f)
{
	uint16_t i;
	for(i=0;i<64;i++)
		{
		fread((void *)(buf+i*0x80), sizeof(uint8_t), 128, f); //read 128 bytes at a time, since there's a hard limit of 255 reads at a time. 64x128 = 8k = 8192 bytes
		}
}

void flipPixels(uint8_t x, uint8_t y, uint8_t count)
{
	uint8_t detect=0;
	uint16_t startPos = TOPLEFT + x + 80*y;
	POKE(MMU_IO_CTRL,2);
	detect = PEEK(startPos);
	for(uint8_t c=0;c<count;c++)
	{
		POKE(startPos+c, detect==SET_PIXEL?CLEAR_PIXEL:SET_PIXEL);
	}
	POKE(MMU_IO_CTRL,0);
}
void setTextColorMatrix()
{
	uint16_t pos = TOPLEFT;
	POKE(MMU_IO_CTRL,3);
	for(uint8_t y=0;y<60;y++)
	{
		for(uint8_t x=0;x<80;x++)
		{
			POKE(pos+x +80*y,0xF0);
		}
	}
	POKE(MMU_IO_CTRL,0);
}
void fillFrame(uint8_t val)
{
	uint16_t pos = TOPLEFT;
	POKE(MMU_IO_CTRL,2);
	for(uint8_t y=0;y<60;y++)
	{
		for(uint8_t x=0;x<80;x++)
		{
			POKE(pos+x +80*y,val);
		}
	}
	POKE(MMU_IO_CTRL,0);
}

int main(int argc, char *argv[]) {
FILE *fileNum =0;
FILE *theMP3file =0;
uint8_t tBuffer[BUFFERSIZE];
uint8_t sBuffer[80*60]; //screen buffer, the characters that are on screen
uint8_t bytesRead;
bool isDone = false;
uint16_t target=0,bTarget=0,sTarget=0;


uint16_t i=0,j=0;
uint16_t bufferIndex=0; //index for the local 8k buffer register
char buffer[CHUNK8K]; //4x the size of the VS FIFO buffer
uint16_t rawFIFOCount=0;
uint16_t bytesToTopOff=0;
uint16_t multipleOf64b = 0;
bool finished = false;
uint8_t deliveredBytes=0;
uint16_t readEntryBufferIndex = 0;

//wipeBitmapBackground(0x2F,0x2F,0x2F);
POKE(MMU_IO_CTRL, 0x00);
	  // XXX GAMMA  SPRITE   TILE  | BITMAP  GRAPH  OVRLY  TEXT
POKE(VKY_MSTR_CTRL_0, 0x01); //sprite,graph,overlay,text
	  // XXX XXX  FON_SET FON_OVLY | MON_SLP DBL_Y  DBL_X  CLK_70
POKE(VKY_MSTR_CTRL_1, 0x00); //font overlay, double height text, 320x240 at 60 Hz;
POKE(MMU_IO_CTRL,0);  //MMU I/O to page 1
POKE(0xD00D, 0x00);

setTextColorMatrix();

fillFrame(32);

openAllCODEC();
boostVSClock();
//initBigPatch();
	
	
//file loading
fileNum = fileOpen("demos/ba_anim.bin","r");

if(fileNum == NULL)
	{
	printf("Can't open demos/ba_anim.bin");
	hitspace();
	return 1;
	}
	
	
theMP3file = fileOpen("demos/ba_audio.mp3","r");

if(theMP3file == NULL)
	{
	printf("Can't open demos/ba_audio.mp3");
	hitspace();
	return 1;
	}
read8KChunk((void *)buffer, theMP3file); //read the first 8k chunk from the .mp3 file
read8KChunk((void *)buffer, theMP3file); //read the first 8k chunk from the .mp3 file
	
//buffer in initial mp3 data
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
	
for(uint16_t e=0; e<80*60;e++)
{
	sBuffer[e]=0xFF;
}
while(!isDone)
	{
	//animation
	target = 0xC000;
	sTarget=0;
	for(uint8_t a=0;a<3;a++) //each frame is 600 bytes, so read every third chunk, 200 bytes
		{
		bTarget=0;
		POKE(MMU_IO_CTRL, 0);
		bytesRead = fileRead(tBuffer, sizeof(uint8_t), 200, fileNum);
		POKE(MMU_IO_CTRL, 2);
		graphicsWaitVerticalBlank();
	
		if(bytesRead == 0) isDone = true;
		else
			{
			for(uint8_t c=0;c<20;c++) //one third of screen
				{
				for(uint8_t b=0;b<10;b++) //one line
					{
					if(sBuffer[sTarget] == tBuffer[bTarget]) {sTarget++; target+=8; bTarget++; continue;}
					sBuffer[sTarget] = tBuffer[bTarget]; //backup
					if((tBuffer[bTarget]&0x80)==0x80) POKE(target++,CLEAR_PIXEL); else POKE(target++,SET_PIXEL);
					if((tBuffer[bTarget]&0x40)==0x40) POKE(target++,CLEAR_PIXEL); else POKE(target++,SET_PIXEL);
					if((tBuffer[bTarget]&0x20)==0x20) POKE(target++,CLEAR_PIXEL); else POKE(target++,SET_PIXEL);
					if((tBuffer[bTarget]&0x10)==0x10) POKE(target++,CLEAR_PIXEL); else POKE(target++,SET_PIXEL);
					if((tBuffer[bTarget]&0x08)==0x08) POKE(target++,CLEAR_PIXEL); else POKE(target++,SET_PIXEL);
					if((tBuffer[bTarget]&0x04)==0x04) POKE(target++,CLEAR_PIXEL); else POKE(target++,SET_PIXEL);
					if((tBuffer[bTarget]&0x02)==0x02) POKE(target++,CLEAR_PIXEL); else POKE(target++,SET_PIXEL);
					if((tBuffer[bTarget]&0x01)==0x01) POKE(target++,CLEAR_PIXEL); else POKE(target++,SET_PIXEL);
					bTarget++;
					sTarget++;
					}
				}
			}

		}
	POKE(MMU_IO_CTRL, 0);
	//audio
	//Check the health of the FIFO buffer
	
	rawFIFOCount = PEEKW(VS_FIFO_COUNT);
	bytesToTopOff = CHUNK2K - (rawFIFOCount&0x07FF); //found how many bytes are left in the 2KB buffer
	multipleOf64b = bytesToTopOff>>6; //multiples of 64 bytes of stuff to top off the FIFO buffer
	
	for(i=0; i<multipleOf64b; i++)
			{
			if(finished) isDone = true;			//make sure we intercept key presses during copying
	
			//Copy a chunk from the application buffer to the VS' buffer
			for(j=0; j<CHUNK64B; j++) {
				POKE(VS_FIFO_DATA, buffer[bufferIndex+j]);
				}

			//advance the application buffer index
			bufferIndex+=CHUNK64B;
			if(bufferIndex == CHUNK8K) bufferIndex = 0; //warp over if the 8KB limit is reached
	
			//read more of the file to replace an equivalent chunk to the application buffer
			deliveredBytes = fread((void *)(buffer+readEntryBufferIndex), sizeof(uint8_t), 64, theMP3file); //Check if we're getting to the end of the file
			if(deliveredBytes !=64)
				{
				//finished = true;
				readEntryBufferIndex+=deliveredBytes;
				}
			else
				{
				readEntryBufferIndex+=CHUNK64B; //keep track of where we're at so we can finish the file
				if(readEntryBufferIndex== CHUNK8K) readEntryBufferIndex = 0; //keep track of where we're at so we can finish the file
				}
			}
	
	}
fileClose(fileNum);
//fileClose(theMP3file);
return 0;}
