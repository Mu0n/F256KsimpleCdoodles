#include "C:\F256\f256llvm-mos\code\myTimer\.builddir\trampoline.h"

#define F256LIB_IMPLEMENTATION

#define TIMER_FRAMES 0
#define TIMER_SECONDS 1

#define TIMER_QUERY_A 0 // 5 frames
#define TIMER_QUERY_B 1 // 20 frames
#define TIMER_QUERY_C 2 // 1s
#define TIMER_QUERY_D 3 // 3s
#define TIMER_QUERY_E 4 // 60s

#define TIMER_DELAY_A 5
#define TIMER_DELAY_B 20
#define TIMER_DELAY_C 1
#define TIMER_DELAY_D 3
#define TIMER_DELAY_E 60

#include "f256lib.h"


struct timer_t myTimers[5]; //timer_t structure for setting timers through the kernel
uint8_t animFrame[5]; // keeps track of which frame it's at in the animation
uint8_t delays[5]; // injects the TIMER_DELAY_x above into a const struct.
	
bool setTimer(const struct timer_t *timer)
{
    *(uint8_t*)0xf3 = timer->units;
    *(uint8_t*)0xf4 = timer->absolute;
    *(uint8_t*)0xf5 = timer->cookie;
    kernelCall(Clock.SetTimer);
	return !kernelError;
}


void textSetup(void)
{
	textClear();
	textSetDouble(true,true);
	textGotoXY(10,5);
	textPrint("- Every 5 frames");
	textGotoXY(10,7);
	textPrint("- Every 20 frames");
	textGotoXY(10,9);
	textPrint("- Every second");
	textGotoXY(10,11);
	textPrint("- Every 3 seconds");
	textGotoXY(10,13);
	textPrint("- Every minute");
}

void timerSetup(void)
{
	uint8_t i=0;
	
	animFrame[0]=1;
	animFrame[1]=1;
	animFrame[2]=1;
	animFrame[3]=1;
	animFrame[4]=1;
	
	delays[0]=TIMER_DELAY_A;
	delays[1]=TIMER_DELAY_B;
	delays[2]=TIMER_DELAY_C;
	delays[3]=TIMER_DELAY_D;
	delays[4]=TIMER_DELAY_E;
	
    myTimers[0].units = TIMER_FRAMES;
	myTimers[1].units = TIMER_FRAMES;
	myTimers[2].units = TIMER_SECONDS;
	myTimers[3].units = TIMER_SECONDS;
	myTimers[4].units = TIMER_SECONDS;
	
	for(i=0;i<5;i++)
	{
		myTimers[i].absolute = delays[i];
	}
	
    myTimers[0].cookie = TIMER_QUERY_A;
    myTimers[1].cookie = TIMER_QUERY_B;
    myTimers[2].cookie = TIMER_QUERY_C;
    myTimers[3].cookie = TIMER_QUERY_D;
    myTimers[4].cookie = TIMER_QUERY_E;
	
	setTimer(&myTimers[0]);
    setTimer(&myTimers[1]);
	setTimer(&myTimers[2]);
	setTimer(&myTimers[3]);
	setTimer(&myTimers[4]);
}

int main(int argc, char *argv[]) {
    byte curTimer;
 
	textSetup();
	timerSetup();
	
    while(true)
        {
        kernelNextEvent();
        if(kernelEventData.type == kernelEvent(timer.EXPIRED))
            {
			curTimer = kernelEventData.timer.cookie; // find out which timer expired through its cookie; use that as an index for everything
			textGotoXY(1,1);
			textPrint("  ");
			textPrintInt(curTimer); //top corner sanity check debug
            textGotoXY(8,5 + 2 * curTimer);
            __putchar(animFrame[curTimer]++); //write the current frame character at the right spot
            if(animFrame[curTimer] > 14) animFrame[curTimer] = 1; //loop back to first anim frame
			myTimers[curTimer].absolute += delays[curTimer]; //prep the next timer
            setTimer(&myTimers[curTimer]); // send it to system
            }
        }
return 0;}
