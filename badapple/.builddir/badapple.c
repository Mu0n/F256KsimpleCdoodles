#include "D:\F256\llvm-mos\code\badapple\.builddir\trampoline.h"

#define F256LIB_IMPLEMENTATION
#include "f256lib.h"
#include "../src/muUtils.h"
#include "../src/muVS1053b.h"
#include "../src/muMP3Play.h"
#include "../src/muFileDiplomacy.h"
#include "../src/muTimer0Int.h"

#define SET_PIXEL 7
#define CLEAR_PIXEL 32
#define TOPLEFT 0xC000
#define BUFFERSIZE 200
#define NIBBLE_TABLE_SIZE 16

static uint8_t expandNibble[NIBBLE_TABLE_SIZE][4];

void initNibbleTable(void)
{
    for (uint8_t v = 0; v < NIBBLE_TABLE_SIZE; v++) {
        expandNibble[v][0] = (v & 0x8) ? CLEAR_PIXEL : SET_PIXEL;
        expandNibble[v][1] = (v & 0x4) ? CLEAR_PIXEL : SET_PIXEL;
        expandNibble[v][2] = (v & 0x2) ? CLEAR_PIXEL : SET_PIXEL;
        expandNibble[v][3] = (v & 0x1) ? CLEAR_PIXEL : SET_PIXEL;
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

static inline void writePixelsForByte(uint8_t value, uint16_t vramAddr)
{
    uint8_t hi = value >> 4;     // upper nibble  (0–15)
    uint8_t lo = value & 0x0F;   // lower nibble  (0–15)

    uint8_t *dst = (uint8_t *)vramAddr;

    const uint8_t *srcHi = expandNibble[hi];
    const uint8_t *srcLo = expandNibble[lo];

    dst[0] = srcHi[0];
    dst[1] = srcHi[1];
    dst[2] = srcHi[2];
    dst[3] = srcHi[3];

    dst[4] = srcLo[0];
    dst[5] = srcLo[1];
    dst[6] = srcLo[2];
    dst[7] = srcLo[3];
}


int main(int argc, char *argv[]) {
FILE *fileNum =0;
FILE *theMP3file =0;
uint8_t tBuffer[BUFFERSIZE];
uint8_t sBuffer[80*60]; //screen buffer, the characters that are on screen
uint8_t bytesRead;
bool isDone = false;
uint16_t target=0,bTarget=0,sTarget=0;
uint16_t frameOff = 0;          // 0, 200, 400, wrap at 600

uint16_t i=0,j=0;
bool finished = false;

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
initNibbleTable();
	
//file loading
fileNum = fileOpen("demos/ba_anim.bin","r");

if(fileNum == NULL)
	{
	printf("Can't open demos/ba_anim.bin");
	hitspace();
	return 1;
	}
	

if(prepMP3ForPlay("demos/ba_audio.mp3"))
	{
	printf("Can't open demos/ba_audio.mp3");
	hitspace();
	return 1;
	}

	
for(uint16_t e=0; e<80*60;e++)
{
	sBuffer[e]=0xFF;
}


setTimer1(1);
while(!isDone)
{
    if(PEEK(INT_PENDING_0) & 0x20)
    {
        POKE(INT_PENDING_0, 0x20);
        setTimer1(1);                       // re-arm right away

        POKE(MMU_IO_CTRL, 0);
        bytesRead = fileRead(tBuffer, 1, 200, fileNum);
        if(bytesRead == 0) { isDone = true; break; }

        POKE(MMU_IO_CTRL, 2);
        uint16_t target = 0xC000 + frameOff * 8;
        for(uint8_t c = 0; c < bytesRead; c++)
        {
            uint8_t nw = tBuffer[c];
            if(sBuffer[frameOff + c] != nw)
            {
                sBuffer[frameOff + c] = nw;
                uint8_t *dst = (uint8_t *)target;
                const uint8_t *h = expandNibble[nw >> 4];
                const uint8_t *l = expandNibble[nw & 0x0F];
                dst[0]=h[0]; dst[1]=h[1]; dst[2]=h[2]; dst[3]=h[3];
                dst[4]=l[0]; dst[5]=l[1]; dst[6]=l[2]; dst[7]=l[3];
            }
            target += 8;
        }
        POKE(MMU_IO_CTRL, 0);

        frameOff += 200;
        if(frameOff >= 600) frameOff = 0;
    }
    if(MP3LoopPass()) break;
}
fileClose(fileNum);
//fileClose(theMP3file);
return 0;}
