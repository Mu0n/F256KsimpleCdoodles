#include "D:\F256\llvm-mos\code\goatt\.builddir\trampoline.h"

#include "f256lib.h"
#include "../src/muUtils.h"

asm (
    ".text                  \n"
    ".global PEEK24         \n"
    "PEEK24:                \n"
    "   sta     $5          \n"
    "   stx     $6          \n"
    "   lda     __rc2       \n"
    "   sta     $7          \n"
    "   ldx     $0          \n"
    "   ldy     $1          \n"
    "   txa                 \n"
    "   ora     #$8         \n"
    "   php                 \n"
    "   sei                 \n"
    "   sta     $0          \n"
    "   tya                 \n"
    "   ora     #48         \n"
    "   sta     $1          \n"
    "   .byte   0xa7,0x05   \n"
    "   stx     $0          \n"
    "   sty     $1          \n"
    "   plp                 \n"
    "   ldx     #0          \n"
    "   rts                 \n"
);

asm (
    ".text                  \n"
    ".global POKE24         \n"
    "POKE24:                \n"
    "   sta     $5          \n"
    "   stx     $6          \n"
    "   lda     __rc2       \n"
    "   sta     $7          \n"
    "   ldx     $0          \n"
    "   ldy     $1          \n"
    "   txa                 \n"
    "   ora     #$8         \n"
    "   php                 \n"
    "   sei                 \n"
    "   sta     $0          \n"
    "   tya                 \n"
    "   ora     #$48         \n"
    "   sta     $1          \n"
    "   lda     __rc4       \n"
    "   .byte   0x87,0x05   \n"
    "   stx     $0          \n"
    "   sty     $1          \n"
    "   plp                 \n"
    "   ldx     #0          \n"
    "   rts                 \n"
);

void lilpause(uint8_t timedelay)
{
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
//will return true if the machine id is detected as a K or K2, enabling peace of mind to use its exclusive features
bool isAnyK(void)
{
	uint8_t value = PEEK(0xD6A7) & 0x1F;
	return (value >= 0x10 && value <= 0x16);
}

bool isK2(void)
{
	uint8_t value = PEEK(0xD6A7) & 0x1F;
	return (value == 0x11);
}

//will return true if the optical keyboard is detected, enabling the case embedded LCD present as well
bool hasCaseLCD(void)
{
	//if the 2nd to last least significant bit is set, it's a mechanical keyboard, LCD not available, don't use it!
	//if it's cleared, then it's an optical keyboard, you can use it!
	return ((PEEK(0xDDC1) & 0x02)==0); //here the bit is cleared, so it's true it's an optical keyboard, it "hasCaseLCD"
}

//returns yes if it's a Jr2 or a K2 in classic mmu mode (ie if it supports a VS1053b or SAM2695)
bool isWave2(void)
{
	uint8_t mid;
	mid = PEEK(0xD6A7)&0x3F;
	return (mid == 0x22 || mid == 0x11); //22 is Jr2 and 11 is K2
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

