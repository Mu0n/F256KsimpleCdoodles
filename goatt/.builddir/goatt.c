#include "D:\F256\llvm-mos\code\goatt\.builddir\trampoline.h"

#define F256LIB_IMPLEMENTATION
#include "f256lib.h"


#include "../src/muUtils.h" //contains helper functions I often use

#include <stdio.h>
#include <stdlib.h>


#define TIMER_FRAMES 0
#define TIMER_SECONDS 1
#define TIMER_SIDPLAY_COOKIE 1

EMBED(sixpack, "../assets/sixpack.bin", 0xA000);

typedef void (*mySIDBin)(void);

void setup()
{
POKE(MMU_IO_CTRL, 0x00);

// XXX GAMMA  SPRITE   TILE  | BITMAP  GRAPH  OVRLY  TEXT
POKE(VKY_MSTR_CTRL_0, 0b00001111); //sprite,graph,overlay,text
// XXX XXX  FON_SET FON_OVLY | MON_SLP DBL_Y  DBL_X  CLK_70
POKE(VKY_MSTR_CTRL_1, 0b00000100); //font overlay, double height text, 320x240 at 60 Hz;
POKE(VKY_LAYER_CTRL_0, 0b00010000); //bitmap 0 in layer 0, bitmap 1 in layer 1
POKE(0xD00D,0x00); //force black graphics background
POKE(0xD00E,0x00);
POKE(0xD00F,0x00);

}


int main(int argc, char *argv[]) {
uint8_t exitFlag = 0;
mySIDBin func = (mySIDBin)0xA000;
mySIDBin playb = (mySIDBin)0xA003;
struct timer_t sidTimer;


setup();


	
printf("Testing out the converted sixpack.sng from beek\n");
hitspace();

func();
sidTimer.units = TIMER_FRAMES;
sidTimer.absolute = getTimerAbsolute(TIMER_FRAMES) + 1;
sidTimer.cookie = TIMER_SIDPLAY_COOKIE;
setTimer(&sidTimer);

while(true)
{
	kernelNextEvent();
	if(kernelEventData.type == kernelEvent(timer.EXPIRED))
	{
		if(kernelEventData.timer.cookie == TIMER_SIDPLAY_COOKIE)
		{
		playb();
		sidTimer.absolute = getTimerAbsolute(TIMER_FRAMES) + 1;
		setTimer(&sidTimer);
		}
	}
	
}

return 0;}
