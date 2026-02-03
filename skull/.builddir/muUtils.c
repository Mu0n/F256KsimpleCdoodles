#include "D:\F256\llvm-mos\code\skull\.builddir\trampoline.h"

#include "f256lib.h"
#include "../src/muUtils.h"


void wipeText()
{
	uint8_t keep = PEEK(MMU_IO_CTRL);
	POKE(MMU_IO_CTRL,2);
	for(uint8_t i=0;i<80;i++)
	{
		for(uint8_t j=0; j<60; j++) POKE(0xC000 + i + j*80, 32);
	}
	POKE(MMU_IO_CTRL,keep);
}

//codec enable all lines
void openAllCODEC()
{
	POKE(0xD620, 0x1F);
	POKE(0xD621, 0x2A);
	POKE(0xD622, 0x01);
	while(PEEK(0xD622) & 0x01);
}

//will return true if the optical keyboard is detected, enabling the case embedded LCD present as well
bool hasCaseLCD(void)
{
	//if the 2nd to last least significant bit is set, it's a mechanical keyboard, LCD not available, don't use it!
	//if it's cleared, then it's an optical keyboard, you can use it!
	return ((PEEK(0xDDC1) & 0x02)==0); //here the bit is cleared, so it's true it's an optical keyboard, it "hasCaseLCD"
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


//simple hit space to continue forced modal delay
void hitspace()
{
	bool exitFlag = false;
	
	while(exitFlag == false)
	{
			kernelNextEvent();
			if(kernelEventData.type == kernelEvent(key.PRESSED))
			{
				switch(kernelEventData.key.raw)
				{
					case 148: //enter
					case 32: //space
						exitFlag = true;
						break;
				}
			}
	}
}
void lilpause(uint8_t timedelay) {
	struct timer_t pauseTimer;
	bool noteExitFlag = false;
	pauseTimer.units = 0; //frames
	pauseTimer.cookie = 213; //let's hope you never use this cookie!
	pauseTimer.absolute = getTimerAbsolute(0) + timedelay;
	setTimer(&pauseTimer);
	noteExitFlag = false;
	while(!noteExitFlag)
	{
		kernelNextEvent();
		if(kernelEventData.type == kernelEvent(timer.EXPIRED))
		{
			switch(kernelEventData.timer.cookie)
			{
			case 213:
				noteExitFlag = true;
				break;
			}
		}
	}
}
