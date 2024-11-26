#include "D:\F256\llvm-mos\code\midiTest\.builddir\trampoline.h"

#include "f256lib.h"
#include "../src/muUtils.h"


//realTextClear: manually changes to MMU page 2 and covers the whole 80x60 text layer
//blank characters. the f256lib.h's textClear seems to only erase part of the screen only.
void realTextClear()
{
	uint16_t c;
	POKE(MMU_IO_CTRL,0x02);
	for(c=0;c<4800;c++)
	{
		POKE(0xC000+c,0x20);
	}
	POKE(MMU_IO_CTRL,0x00);
}

//Sends a kernel based timer. You must prepare a timer_t struct first and initialize its fields
bool setTimer(const struct timer_t *timer)
{
    *(uint8_t*)0xf3 = timer->units;
    *(uint8_t*)0xf4 = timer->absolute;
    *(uint8_t*)0xf5 = timer->cookie;
    kernelCall(Clock.SetTimer);
	return !kernelError;
}
//getTimerAbsolute:
//This is essential if you want to retrigger a timer properly. The old value of the absolute
//field has a high chance of being desynchronized when you arrive at the moment when a timer
//is expired and you must act upon it.
//get the value returned by this, add the delay you want, and use setTimer to send it off
//ex: myTimer.absolute = getTimerAbsolute(TIMES_SECONDS) + TIMER_MYTIMER_DELAY
uint8_t getTimerAbsolute(uint8_t units)
{
    *(uint8_t*)0xf3 = units | 0x80;
    return kernelCall(Clock.SetTimer);
}

//injectChar: injects a specific character in a specific location on screen.
//position x,y is where it'll be injected in text layer coordinates
//theChar is the byte from 0-255 that will be placed there
//col(umn) should be either 40 (in double character width mode) or 80 (in regular width mode)
void injectChar40col(uint8_t x, uint8_t y, uint8_t theChar, uint8_t col)
{
		POKE(MMU_IO_CTRL,0x02); //set io_ctrl to 2 to access text screen
		POKE(0xC000 + col * y + x, theChar);
		POKE(MMU_IO_CTRL,0x00);  //set it back to default
}

