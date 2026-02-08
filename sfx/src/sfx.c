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

#define SFX01_CAM_BASE  0x10000
#define SFX01_CAM_END   0x116FC
#define SFX02_BONG_BASE 0x20000
#define SFX02_BONG_END  0x2184C

bool nearingEnd = false;
bool hasEnded = false;
bool isPlaying = false;

EMBED(camera, "../assets/camera.mp3", 0x10000);//21,504 bytes
EMBED(bong, "../assets/blip.mp3", 0x20000);//6,220 bytes

uint32_t sfxPtr01 = SFX01_CAM_BASE;
uint32_t sfxPtr02 = SFX02_BONG_BASE;

__attribute__((optnone))
void read2KChunk()
{
	
	if(hasEnded) return;
	
	if(nearingEnd) //is in the end phase
		{
		for(uint8_t i=0;i<32;i++) //do this 32 times * 64 bytes = 2048 bytes
			{
			while(2048 - (PEEKW(VS_FIFO_COUNT)&0x07FF) < 64)
				; //wait for a 64 bytes space that's free in the FIFO at least
				
			for(uint8_t j=0;j<64;j++)	POKE(VS_FIFO_DATA, 0);	//empty out the next 64 bytes
			}
			
		nearingEnd = false;
		isPlaying = false;
		hasEnded = true;
		return;
		}
		
	for(uint8_t i=0;i<64;i++)
		{
		if(sfxPtr01 < (SFX01_CAM_END - 32)) 
		{
			while(2048 - (PEEKW(VS_FIFO_COUNT)&0x07FF) < 32)
				;
			
			for(uint8_t j=0;j<32;j++) 
			{
				POKE(VS_FIFO_DATA, FAR_PEEK(sfxPtr01++));
			}
		}
		else //starting to reach the end
			{
			uint16_t howManyLeft = (uint16_t)(SFX01_CAM_END - sfxPtr01);
			while(2048 - (PEEKW(VS_FIFO_COUNT)&0x07FF) < howManyLeft)
				;
			
			
			for(uint8_t j=0;j<howManyLeft;j++) 
			{
				POKE(VS_FIFO_DATA, FAR_PEEK(sfxPtr01++));
			}
			nearingEnd = true;
			lowVol();
			return;
			}
		}
}

__attribute__((optnone))
int main(int argc, char *argv[]) {
//wipeBitmapBackground(0x2F,0x2F,0x2F);
POKE(MMU_IO_CTRL, 0x00);
	  // XXX GAMMA  SPRITE   TILE  | BITMAP  GRAPH  OVRLY  TEXT
POKE(VKY_MSTR_CTRL_0, 0x01); //sprite,graph,overlay,text
	  // XXX XXX  FON_SET FON_OVLY | MON_SLP DBL_Y  DBL_X  CLK_70
POKE(VKY_MSTR_CTRL_1, 0x00); //font overlay, double height text, 320x240 at 60 Hz;
POKE(MMU_IO_CTRL,0);  //MMU I/O to page 1
POKE(0xD00D, 0x00);

openAllCODEC();
//initBigPatch();
boostVSClock();

//file loading

highVol();
	
while(hasEnded == false) //sound it the first time
	{	
	read2KChunk(); 
	}

printf("\nPress a key for a sound");	
while(true)
	{
	if(isPlaying)
		{
		read2KChunk(); 
		}
		
	kernelNextEvent();
	if(kernelEventData.type == kernelEvent(key.PRESSED) && hasEnded)
		{	
		sfxPtr01 = SFX01_CAM_BASE;
		sfxPtr02 = SFX02_BONG_BASE;
		
		hasEnded = false;
		nearingEnd = false;
		isPlaying = true;
		}	
	}
return 0;}
}