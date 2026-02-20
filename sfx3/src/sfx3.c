#define F256LIB_IMPLEMENTATION
#include "f256lib.h"
#include "../src/muUtils.h"
#include "../src/muVS1053b.h"
#include "../src/muMidiPlay2.h"
#include "../src/muMidi.h"
#include "../src/muTimer0Int.h"


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
#define SFX02_BONG_BASE 0x20000
#define SFX03_COIN_BASE 0x30000
#define SFX04_TIEF_BASE 0x40000

#define MUSIC_BASE 0x50000

#define WAV_HEADER_OFFSET 0x2C
#define SILENTBYTE        0x80

#define DMA_16BIT_MODE    0x40

bool nearingEnd = false;
bool isPlaying = false;

EMBED(camera, "../assets/camera.wav", 0x10000);//14,862 bytes
EMBED(bong, "../assets/blip.wav", 0x20000);//11,598 bytes
EMBED(coin, "../assets/coin.wav", 0x30000);//7,174 bytes
EMBED(tie, "../assets/tie.wav", 0x40000);//23132 bytes
EMBED(canyon, "../assets/canyon.mid", 0x50000);

typedef struct pcmChannel{
uint32_t startAddr; //sample in far memory
uint16_t size; //total size
uint8_t *bufferAddr; //attributed buffer start in near memory
uint16_t offset; //offset in near memory
uint16_t farOffset; //offset in far memory

bool isPlaying;
} pCh, *pChPtr;


pCh sfxChannels[4];
uint8_t data0,data1,data2,data3;

//lower memory sound buffers
static uint8_t chan0[2048]; //2kb buffer for channel 0
static uint8_t chan1[2048]; //2kb buffer for channel 1
static uint8_t chan2[2048]; //2kb buffer for channel 2
static uint8_t chan3[2048]; //2kb buffer for channel 3


void setupSound(uint8_t, uint32_t, uint16_t, uint16_t);
void startSound(uint8_t);
void stopSound(uint8_t);
uint8_t mix_n(uint8_t);



void dmaCopy16(uint32_t source, uint32_t dest, uint32_t length) {
	while (PEEKW(RAST_ROW_L) < 482); // Wait for VBL.

	POKE(DMA_CTRL, DMA_CTRL_ENABLE | DMA_16BIT_MODE); //set in 16 bit mode
	//POKEW(DMA_FILL_VAL, 0x0000); //clear the fill value
	POKEA(DMA_SRC_ADDR, source);
	POKEA(DMA_DST_ADDR, dest);
	POKEA(DMA_COUNT, length); //number of bytes should be even number to be coherent with a group of 16 bytes
	POKE(DMA_CTRL, PEEK(DMA_CTRL) | DMA_CTRL_START);

//	dmaWait();
}


void setupSound(uint8_t which, uint32_t start, uint16_t size, uint16_t bufferAddr)
{
	sfxChannels[which].startAddr = start+(uint32_t)WAV_HEADER_OFFSET;
	sfxChannels[which].size   = size-(uint32_t)WAV_HEADER_OFFSET;
	sfxChannels[which].bufferAddr = (uint8_t *)bufferAddr;
	sfxChannels[which].offset = 0;
	sfxChannels[which].farOffset = 0;

	dmaCopy16(sfxChannels[which].startAddr,(uint32_t)bufferAddr,2048);
}

void startSound(uint8_t which)
{
	sfxChannels[which].offset = 0;
	sfxChannels[which].farOffset = 0;
	sfxChannels[which].isPlaying=true;
}

void stopSound(uint8_t which)
{
	sfxChannels[which].offset = 0;
	sfxChannels[which].farOffset = 0;
	sfxChannels[which].isPlaying=false;
}


const uint8_t pcmHeader[]=
	{
	0x52, 0x49, 0x46, 0x46, 0x06, 0x3a, 0x00, 0x00, 0x57, 0x41, 0x56, 0x45, 0x66, 0x6d, 0x74, 0x20,
	0x10, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x22, 0x56, 0x00, 0x00, 0x22, 0x56, 0x00, 0x00,
	0x01, 0x00, 0x08, 0x00, 0x64, 0x61, 0x74, 0x61, 0xe2, 0x39, 0x00, 0x00
	}; //header for 8-bit, unsigned, 22.05kHz wav PCM files
	
bool doing2 = false;

uint8_t mix8(uint8_t a, uint8_t b)
{
    int s1 = (int)a - 128;
    int s2 = (int)b - 128;

    // Soft mix: avoids clipping and keeps volume stable
    int m = (s1 + s2)/2;

    // Clamp
    if (m > 127) m = 127;
    if (m < -128) m = -128;

    return (uint8_t)(m + 128);
}


uint8_t mix_n(uint8_t nbSounds)
{
	int16_t sum=0;
	
    if (nbSounds == 0) return 128; // silence

	if(sfxChannels[0].isPlaying) sum += (int)data0 - 128;
	if(sfxChannels[1].isPlaying) sum += (int)data1 - 128;
	if(sfxChannels[2].isPlaying) sum += (int)data2 - 128;
	if(sfxChannels[3].isPlaying) sum += (int)data3 - 128;

    int mixed = mathSignedDivision(sum,nbSounds);


    return (uint8_t)(mixed + 128);
}

void headerPump()
{
	for(uint8_t i=0;i<WAV_HEADER_OFFSET;i++) POKE(VS_FIFO_DATA, pcmHeader[i]);
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
void playChunk()
{
	int16_t nbSounds = sfxChannels[0].isPlaying ? 1:0 + sfxChannels[1].isPlaying ? 1:0 + sfxChannels[2].isPlaying ? 1:0 + sfxChannels[3].isPlaying ? 1:0;
	
	if(nbSounds==0) return; //no active sound, quit
	while((2048 - (PEEKW(VS_FIFO_COUNT)&0x07FF)) < 1024)
		; 
	//printf(".");
	for(uint16_t z=0;z<1024;z++) //64 bytes at a time
	{
			if(sfxChannels[0].isPlaying)  
			{
			//data0 = FAR_PEEK(sfxChannels[0].startAddr + (uint32_t)sfxChannels[0].offset++);
			data0 = PEEK(sfxChannels[0].bufferAddr + sfxChannels[0].offset++);
			sfxChannels[0].farOffset++;
			
			if(sfxChannels[0].farOffset == sfxChannels[0].size) //check if reached end
				{
				sfxChannels[0].isPlaying = false;
				}
			

			}
			else data0=SILENTBYTE;
			
			if(sfxChannels[1].isPlaying)  
			{
			//data1 = FAR_PEEK(sfxChannels[1].startAddr + (uint32_t)sfxChannels[1].offset++);
			data1 = PEEK(sfxChannels[1].bufferAddr + sfxChannels[1].offset++);
			sfxChannels[1].farOffset++;
			if(sfxChannels[1].farOffset  == sfxChannels[1].size) 
				{
					sfxChannels[1].isPlaying = false;
				}
			}
			else data1=SILENTBYTE;

			if(sfxChannels[2].isPlaying)  
			{
			//data2 = FAR_PEEK(sfxChannels[2].startAddr + (uint32_t)sfxChannels[2].offset++);
			data2 = PEEK(sfxChannels[2].bufferAddr + sfxChannels[2].offset++);
			sfxChannels[2].farOffset++;
			if(sfxChannels[2].farOffset  == sfxChannels[2].size) 
				{
					sfxChannels[2].isPlaying = false;
				}
			}
			else data2=SILENTBYTE;
						
			if(sfxChannels[3].isPlaying)  
			{
			//data3 = FAR_PEEK(sfxChannels[3].startAddr + (uint32_t)sfxChannels[3].offset++);
			data3 = PEEK(sfxChannels[3].bufferAddr + sfxChannels[3].offset++);
			sfxChannels[3].farOffset++;
			if(sfxChannels[3].farOffset  == sfxChannels[3].size) 
				{
					sfxChannels[3].isPlaying = false;
				}
			}
			else data3=SILENTBYTE;

	//POKE(VS_FIFO_DATA, (uint8_t)(((((int)data0)-128)   + (((int)data1)-128))/2+128)     );
	POKE(VS_FIFO_DATA, mix_n(nbSounds));
	
	}
	
	//what to do after 64 bytes have been processed
	if(sfxChannels[0].isPlaying && sfxChannels[0].offset == 2048)
	{
		sfxChannels[0].offset=0;
		dmaCopy16(sfxChannels[0].startAddr+(uint32_t)sfxChannels[0].farOffset,(uint32_t)chan0,2048);
	}
	if(sfxChannels[1].isPlaying && sfxChannels[1].offset == 2048)
	{
		sfxChannels[1].offset=0;
		dmaCopy16(sfxChannels[1].startAddr+(uint32_t)sfxChannels[1].farOffset,(uint32_t)chan1,2048);
	}
	if(sfxChannels[2].isPlaying && sfxChannels[2].offset == 2048)
	{
		sfxChannels[2].offset=0;
		dmaCopy16(sfxChannels[2].startAddr+(uint32_t)sfxChannels[2].farOffset,(uint32_t)chan2,2048);
	}
	if(sfxChannels[3].isPlaying && sfxChannels[3].offset == 2048)
	{
		sfxChannels[3].offset=0;
		dmaCopy16(sfxChannels[3].startAddr+(uint32_t)sfxChannels[3].farOffset,(uint32_t)chan3,2048);
	}

}

__attribute__((optnone))
int main(int argc, char *argv[]) {
bool isMIDIActive = false;
	
//wipeBitmapBackground(0x2F,0x2F,0x2F);
POKE(MMU_IO_CTRL, 0x00);
	  // XXX GAMMA  SPRITE   TILE  | BITMAP  GRAPH  OVRLY  TEXT
POKE(VKY_MSTR_CTRL_0, 0x01); //sprite,graph,overlay,text
	  // XXX XXX  FON_SET FON_OVLY | MON_SLP DBL_Y  DBL_X  CLK_70
POKE(VKY_MSTR_CTRL_1, 0x00); //font overlay, double height text, 320x240 at 60 Hz;
POKE(MMU_IO_CTRL,0);  //MMU I/O to page 1
POKE(0xD00D, 0x00);

openAllCODEC();
boostVSClock();

//initial sfx setup
setupSound(0, SFX01_CAM_BASE,  14862, (uint16_t)chan0);
setupSound(1, SFX02_BONG_BASE, 11598, (uint16_t)chan1);
setupSound(2, SFX03_COIN_BASE, 7174,  (uint16_t)chan2);
setupSound(3, SFX04_TIEF_BASE, 23132, (uint16_t)chan3);

headerPump();

lowVol();
silencePad();
highVol();	
midiShutUp(0);

initTrack(MUSIC_BASE);
resetTimer0();



printf("copy done addr=%04x %04x %04x %04x",chan0, chan1, chan2, chan3);

printf("\nSpace to toggle MIDI playback\nPress 1 for [CAMERA SHUTTER]\nPress 2 for [BOING]\nPress 3 for [COIN]\nPress 4 for [TIEF]\n\n");	
while(true)
	{
	
	playChunk(); //always play something but verify inside
	if(isMIDIActive)
	{
		if(PEEK(INT_PENDING_0)&0x10) //when the timer0 delay is up, go here
			{
			POKE(INT_PENDING_0,0x10); //clear the timer0 delay
			playMidi(); //play the next chunk of the midi file, might deal with multiple 0 delay stuff
			}
		if(theOne.isWaiting == false) 
			{
			sniffNextMIDI(); //find next event to play, will cue up a timer0 delay
			}
	}
				
	kernelNextEvent();
	if(kernelEventData.type == kernelEvent(key.PRESSED))
		{	
		switch(kernelEventData.key.raw)
			{
			case 0x31:
					printf("1");
					startSound(0);
					dmaCopy16(sfxChannels[0].startAddr, (uint32_t)chan0,2048);
				break;
			case 0x32:
					printf("2");
					startSound(1);
					dmaCopy16(sfxChannels[1].startAddr,(uint32_t)chan1,2048);
				break;
			case 0x33:
					printf("3");
					startSound(2);
					dmaCopy16(sfxChannels[2].startAddr,(uint32_t)chan2,2048);
				break;
			case 0x34:
					printf("4");
					startSound(3);
					dmaCopy16(sfxChannels[3].startAddr,(uint32_t)chan3,2048);
				break;
			case 0x20: //space
					isMIDIActive = !isMIDIActive;
					if(isMIDIActive==false) midiShutAllChannels(0);
				break;
			}
		}	
	}
return 0;}
}