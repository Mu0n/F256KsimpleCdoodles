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
#define SFX01_CAM_END   0x13A0E
#define SFX02_BONG_BASE 0x20000
#define SFX02_BONG_END  0x241A6

bool nearingEnd = false;
bool hasEnded = false;
bool isPlaying = false;

EMBED(camera, "../assets/camera.wav", 0x10000);//14,862 bytes
EMBED(bong, "../assets/blip.wav", 0x20000);//16,806 bytes

const uint8_t header[]=
	{
	0x52, 0x49, 0x46, 0x46, 0x06, 0x3a, 0x00, 0x00,
	0x57, 0x41, 0x56, 0x45, 0x66, 0x6d, 0x74, 0x20,
	0x10, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 
	0x22, 0x56, 0x00, 0x00, 0x22, 0x56, 0x00, 0x00,
	0x01, 0x00, 0x08, 0x00, 0x64, 0x61, 0x74, 0x61,
	0xe2, 0x39, 0x00, 0x00
	};

uint32_t sfxPtr01 = SFX01_CAM_BASE+0x2c;
uint32_t sfxPtr02 = SFX02_BONG_BASE+0x2c;
bool doing2 = false;

uint8_t mix8(uint8_t a, uint8_t b)
{
    int s1 = (int)a - 128;
    int s2 = (int)b - 128;

    int m = s1 + s2;

    if(m > 127) m = 127;
    if(m < -128) m = -128;

    return (uint8_t)(m + 128);
}

void headerPump()
{
	for(uint8_t i=0;i<44;i++) POKE(VS_FIFO_DATA, header[i]);
}
void silencePad()
{
	for(uint8_t i=0;i<32;i++) //do this 32 times * 64 bytes = 2048 bytes
	{
	while(2048 - (PEEKW(VS_FIFO_COUNT)&0x07FF) < 64)
		; //wait for a 64 bytes space that's free in the FIFO at least
		
	for(uint8_t j=0;j<64;j++)	POKE(VS_FIFO_DATA, 0x80);	//empty out the next 64 bytes
	}
}

__attribute__((optnone))
void read2KChunk()
{
	
	if(hasEnded) return;
	
	if(nearingEnd) //is in the end phase
		{
			
		silencePad();
			
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
				uint8_t data = FAR_PEEK(sfxPtr01++);
				uint8_t data2 = FAR_PEEK(sfxPtr02++);
				
				if(doing2) data = mix8(data,data2);
				
				POKE(VS_FIFO_DATA, data);
			}
		}
		else //starting to reach the end
			{
			uint16_t howManyLeft = (uint16_t)(SFX01_CAM_END - sfxPtr01);
			while(2048 - (PEEKW(VS_FIFO_COUNT)&0x07FF) < howManyLeft)
				;
			
			for(uint8_t j=0;j<howManyLeft;j++) 
			{
				uint8_t data = FAR_PEEK(sfxPtr01++);
				uint8_t data2 = FAR_PEEK(sfxPtr02++);
				if(doing2) data = mix8(data,data2);
				POKE(VS_FIFO_DATA, data);
			}
			nearingEnd = true;
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
	
doing2 = true;
headerPump();	
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
	printf(".");
		sfxPtr01 = SFX01_CAM_BASE+0x2c;
		sfxPtr02 = SFX02_BONG_BASE+0x2c;
		
		lowVol();
		silencePad();
		highVol();
		
		hasEnded = false;
		nearingEnd = false;
		isPlaying = true;
		
		}	
	}
return 0;}
}