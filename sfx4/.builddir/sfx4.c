#include "D:\F256\llvm-mos\code\sfx4\.builddir\trampoline.h"

#define F256LIB_IMPLEMENTATION
#include "f256lib.h"
#include "../src/muUtils.h"
#include "../src/muVS1053b.h"
#include "../src/muMidiPlay2.h"
#include "../src/muMidi.h"
#include "../src/muTimer0Int.h"
#include "../src/mupsg.h"
#include "../src/musid.h"


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

bool nearingEnd = false;
bool isPlaying = false;

EMBED(camera, "../assets/camera.wav", 0x10000);//14,862 bytes
EMBED(bong, "../assets/blip.wav", 0x20000);//11,598 bytes
EMBED(coin, "../assets/coin.wav", 0x30000);//7,174 bytes
EMBED(tie, "../assets/tie.wav", 0x40000);//23132 bytes
EMBED(canyon, "../assets/canyon.mid", 0x50000);

typedef struct pcmChannel{
uint32_t startAddr;
uint16_t size;
uint16_t offset;
bool isPlaying;
} pCh, *pChPtr;


pCh sfxChannels[4];
uint8_t data0,data1,data2,data3;

void setupSound(uint8_t, uint32_t, uint16_t);
void startSound(uint8_t);
void stopSound(uint8_t);
uint8_t mix_n(void);

void setupSound(uint8_t which, uint32_t start, uint16_t size)
{
	sfxChannels[which].startAddr = start+(uint32_t)WAV_HEADER_OFFSET;
	sfxChannels[which].size   = size-(uint32_t)WAV_HEADER_OFFSET;
	sfxChannels[which].offset = 0;
}

void startSound(uint8_t which)
{
	sfxChannels[which].offset = 0;
	sfxChannels[which].isPlaying=true;
}

void stopSound(uint8_t which)
{
	sfxChannels[which].offset = 0;
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

int16_t howManySounds()
{
	return sfxChannels[0].isPlaying ? 1:0 + sfxChannels[1].isPlaying ? 1:0 + sfxChannels[2].isPlaying ? 1:0 + sfxChannels[3].isPlaying ? 1:0;
}
uint8_t mix_n()
{
	int16_t sum=0;
	int16_t nbSounds = howManySounds();
	
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
void playSIDChunk()
{
	int16_t nbSounds = howManySounds();
	if(nbSounds==0) return; //no active sound, quit
	for(uint8_t z=0;z<64;z++)
	{
	
	
	if(sfxChannels[0].isPlaying)
	{
	data0 = FAR_PEEK(sfxChannels[0].startAddr + (uint32_t)sfxChannels[0].offset++);
	if(sfxChannels[0].offset == sfxChannels[0].size)
		{
			sfxChannels[0].isPlaying = false;
			shutAllSIDVoices();
			return;
		}
	}
	else if(sfxChannels[1].isPlaying)
	{
	data0 = FAR_PEEK(sfxChannels[1].startAddr + (uint32_t)sfxChannels[1].offset++);
	if(sfxChannels[1].offset == sfxChannels[1].size)
		{
			sfxChannels[1].isPlaying = false;
			shutAllSIDVoices();
			return;
		}
	}
	else if(sfxChannels[2].isPlaying)
	{
	data0 = FAR_PEEK(sfxChannels[2].startAddr + (uint32_t)sfxChannels[2].offset++);
	if(sfxChannels[2].offset == sfxChannels[2].size)
		{
			sfxChannels[2].isPlaying = false;
			shutAllSIDVoices();
			return;
		}
	}
	else if(sfxChannels[3].isPlaying)
	{
	data0 = FAR_PEEK(sfxChannels[3].startAddr + (uint32_t)sfxChannels[3].offset++);
	if(sfxChannels[3].offset == sfxChannels[3].size)
		{
			sfxChannels[3].isPlaying = false;
			shutAllSIDVoices();
			return;
		}
	}
	else data0=0x00;
	
	uint16_t pwm = (data0-128)<<4;
	
	setTimer0(475);
	while(true)
		{
		if(PEEK(INT_PENDING_0)&0x10) //when the timer0 delay is up, go here
				{
	POKEW(SID1+(uint16_t)SID_LO_PWDC, pwm); // SET PULSE WAVE DUTY LOW BYTE
				POKE(INT_PENDING_0,0x10); //clear the timer0 delay
				break;
				}
		}

	}
}

__attribute__((optnone))
void playPSGChunk()
{
	int16_t nbSounds = howManySounds();
	if(nbSounds==0) return; //no active sound, quit
	for(uint8_t z=0;z<64;z++)
	{
	
	
	if(sfxChannels[0].isPlaying)
	{
	data0 = FAR_PEEK(sfxChannels[0].startAddr + (uint32_t)sfxChannels[0].offset++);
	if(sfxChannels[0].offset == sfxChannels[0].size)
		{
			sfxChannels[0].isPlaying = false;
			shutPSG();
			return;
		}
	}
	else if(sfxChannels[1].isPlaying)
	{
	data0 = FAR_PEEK(sfxChannels[1].startAddr + (uint32_t)sfxChannels[1].offset++);
	if(sfxChannels[1].offset == sfxChannels[1].size)
		{
			sfxChannels[1].isPlaying = false;
			shutPSG();
			return;
		}
	}
	else if(sfxChannels[2].isPlaying)
	{
	data0 = FAR_PEEK(sfxChannels[2].startAddr + (uint32_t)sfxChannels[2].offset++);
	if(sfxChannels[2].offset == sfxChannels[2].size)
		{
			sfxChannels[2].isPlaying = false;
			shutPSG();
			return;
		}
	}
	else if(sfxChannels[3].isPlaying)
	{
	data0 = FAR_PEEK(sfxChannels[3].startAddr + (uint32_t)sfxChannels[3].offset++);
	if(sfxChannels[3].offset == sfxChannels[3].size)
		{
			sfxChannels[3].isPlaying = false;
			shutPSG();
			return;
		}
	}
	else data0=0x00;
	
	data0=15-((data0-128)>>3);
	
	setTimer0(475);
	while(true)
		{
		if(PEEK(INT_PENDING_0)&0x10) //when the timer0 delay is up, go here
				{
				POKE(PSG_LEFT,0x90 | data0);
				POKE(INT_PENDING_0,0x10); //clear the timer0 delay
				break;
				}
		}

	}
}

__attribute__((optnone))
void playChunk()
{
	int16_t nbSounds = howManySounds();
	if(nbSounds==0) return; //no active sound, quit
	while((2048 - (PEEKW(VS_FIFO_COUNT)&0x07FF)) < 64)
		;
	//printf(".");
	for(uint8_t z=0;z<64;z++)
	{
			if(sfxChannels[0].isPlaying)
			{
			data0 = FAR_PEEK(sfxChannels[0].startAddr + (uint32_t)sfxChannels[0].offset++);
			if(sfxChannels[0].offset == sfxChannels[0].size)
				{
					sfxChannels[0].isPlaying = false;
				}
			}
			else data0=SILENTBYTE;
	
			if(sfxChannels[1].isPlaying)
			{
			data1 = FAR_PEEK(sfxChannels[1].startAddr + (uint32_t)sfxChannels[1].offset++);
			if(sfxChannels[1].offset == sfxChannels[1].size)
				{
					sfxChannels[1].isPlaying = false;
				}
			}
			else data1=SILENTBYTE;

			if(sfxChannels[2].isPlaying)
			{
			data2 = FAR_PEEK(sfxChannels[2].startAddr + (uint32_t)sfxChannels[2].offset++);
			if(sfxChannels[2].offset == sfxChannels[2].size)
				{
					sfxChannels[2].isPlaying = false;
				}
			}
			else data2=SILENTBYTE;
	
			if(sfxChannels[3].isPlaying)
			{
			data3 = FAR_PEEK(sfxChannels[3].startAddr + (uint32_t)sfxChannels[3].offset++);
			if(sfxChannels[3].offset == sfxChannels[3].size)
				{
					sfxChannels[3].isPlaying = false;
				}
			}
			else data3=SILENTBYTE;

	//POKE(VS_FIFO_DATA, (uint8_t)(((((int)data0)-128)   + (((int)data1)-128))/2+128)     );
	POKE(VS_FIFO_DATA, mix_n());
	
	}
}

__attribute__((optnone))
int main(int argc, char *argv[]) {
bool isMIDIActive = false;
uint8_t chipChoice = 0;
	
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

setupSound(0, SFX01_CAM_BASE,  14862);
setupSound(1, SFX02_BONG_BASE, 11598);
setupSound(2, SFX03_COIN_BASE, 7174);
setupSound(3, SFX04_TIEF_BASE, 23132);
	
headerPump();

lowVol();
silencePad();
highVol();
midiShutUp(0);
setStereoPSG();
shutPSG();
shutAllSIDVoices();

	POKE(SID1+SID_LO_B,0xFF);
	POKE(SID1+SID_HI_B,0xFF);
	POKE(SID1+(uint16_t)SID_LO_PWDC, 0x00); // SET PULSE WAVE DUTY LOW BYTE
	POKE(SID1+(uint16_t)SID_HI_PWDC, 0x04); // SET PULSE WAVE DUTY HIGH BYTE
	POKE(SID1+(uint16_t)SID_ATK_DEC, 0x0F); // SET ATTACK;DECAY
	POKE(SID1+(uint16_t)SID_SUS_REL, 0xff); // SET SUSTAIN;RELEASE
	POKE(SID1+(uint16_t)SID_CTRL, 0x40); 	 // SET CTRL as
	POKE(SID1+SID_FM_VC,0x04);
	

initTrack(MUSIC_BASE);
resetTimer0();

printf("\nSpace to toggle MIDI playback\n");
printf("\n[CAMERA SHUTTER] 1: VS1053b  Q: PSG (4bit vol)  A: SID: (12bit pwm)");
printf("\n         [BOING] 2: VS1053b  W: PSG (4bit vol)  S: SID: (12bit pwm)");
printf("\n          [COIN] 3: VS1053b  E: PSG (4bit vol)  D: SID: (12bit pwm)");
printf("\n          [TIEF] 4: VS1053b  R: PSG (4bit vol)  F: SID: (12bit pwm)\n\n");
while(true)
	{
	
	if(chipChoice==0) playChunk(); //always play something but verify inside
	else if(chipChoice==1)playPSGChunk();
	else if(chipChoice==2)playSIDChunk();
	
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
					chipChoice=0;
				break;
			case 0x32:
					printf("2");
					startSound(1);
					chipChoice=0;
				break;
			case 0x33:
					printf("3");
					startSound(2);
					chipChoice=0;
				break;
			case 0x34:
					printf("4");
					startSound(3);
					chipChoice=0;
				break;
	
	
			case 0x71:
					printf("Q");
					psgNoteOn(0, PSG_LEFT, 0x80, 0x00,0x00);
					startSound(0);
					chipChoice=1;
				break;
			case 0x77:
					printf("W");
					psgNoteOn(0, PSG_LEFT, 0x80,0x00,0x00);
					startSound(1);
					chipChoice=1;
				break;
			case 0x65:
					printf("E");
					psgNoteOn(0, PSG_LEFT, 0x80,0x00,0x00);
					startSound(2);
					chipChoice=1;
				break;
			case 0x72:
					printf("R");
					psgNoteOn(0, PSG_LEFT, 0x80,0x00,0x00);
					startSound(3);
					chipChoice=1;
				break;
	
			case 0x61:
					printf("A");
					startSound(0);
					sidNoteOnOrOff(SID1+SID_VOICE1+SID_CTRL, 0x41, true);
					chipChoice=2;
				break;
			case 0x73:
					printf("S");
					startSound(1);
					sidNoteOnOrOff(SID1+SID_VOICE1+SID_CTRL, 0x41, true);
					chipChoice=2;
				break;
			case 0x64:
					printf("D");
					startSound(2);
					sidNoteOnOrOff(SID1+SID_VOICE1+SID_CTRL, 0x41, true);
					chipChoice=2;
				break;
			case 0x66:
					printf("F");
					startSound(3);
					sidNoteOnOrOff(SID1+SID_VOICE1+SID_CTRL, 0x41, true);
					chipChoice=2;
				break;
			case 0x20:
					isMIDIActive = !isMIDIActive;
					if(isMIDIActive==false) midiShutAllChannels(0);
				break;
			}
		}
	}
return 0;}
