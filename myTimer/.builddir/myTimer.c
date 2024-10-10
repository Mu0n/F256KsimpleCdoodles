#include "D:\F256\llvm-mos\code\myTimer\.builddir\trampoline.h"

#define F256LIB_IMPLEMENTATION

#define TIMER_FRAMES 0
#define TIMER_SECONDS 1

#define TIMER_QUERY_A 0 // 1 frames
#define TIMER_QUERY_B 1 // 5 frames
#define TIMER_QUERY_C 2 // 20 frames
#define TIMER_QUERY_D 3 // 1s
#define TIMER_QUERY_E 4 // 3s
#define TIMER_QUERY_F 5 // 60s

#define TIMER_DELAY_A 1
#define TIMER_DELAY_B 5
#define TIMER_DELAY_C 20
#define TIMER_DELAY_D 1
#define TIMER_DELAY_E 3
#define TIMER_DELAY_F 60

#include "f256lib.h"


struct timer_t myTimers[6]; //timer_t structure for setting timers through the kernel
uint8_t animFrame[6]; // keeps track of which frame it's at in the animation
uint8_t delays[6]; // injects the TIMER_DELAY_x above into a const struct.
	
void injectChar(uint8_t x, uint8_t y, uint8_t theChar)
{
		POKE(0x0001,0x02); //set io_ctrl to 2 to access text screen
		POKE(0xC000 + 40 * y + x, theChar);
		POKE(0x0001,0x00);  //set it back to default
}

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
	textGotoXY(10,3);
	textPrint("- Every frame");
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
	animFrame[5]=1;
	
	
	
	delays[0]=TIMER_DELAY_A;
	delays[1]=TIMER_DELAY_B;
	delays[2]=TIMER_DELAY_C;
	delays[3]=TIMER_DELAY_D;
	delays[4]=TIMER_DELAY_E;
	delays[5]=TIMER_DELAY_F;
	
    myTimers[0].units = TIMER_FRAMES;
	myTimers[1].units = TIMER_FRAMES;
	myTimers[2].units = TIMER_FRAMES;
	myTimers[3].units = TIMER_SECONDS;
	myTimers[4].units = TIMER_SECONDS;
    myTimers[5].units = TIMER_SECONDS;
	
	kernelNextEvent();
	
	for(i=0;i<6;i++)
	{
		myTimers[i].absolute = kernelEventData.timer + delays[i];
	}
	
    myTimers[0].cookie = TIMER_QUERY_A;
    myTimers[1].cookie = TIMER_QUERY_B;
    myTimers[2].cookie = TIMER_QUERY_C;
    myTimers[3].cookie = TIMER_QUERY_D;
    myTimers[4].cookie = TIMER_QUERY_E;
    myTimers[5].cookie = TIMER_QUERY_F;
	
	setTimer(&myTimers[0]);
    setTimer(&myTimers[1]);
	setTimer(&myTimers[2]);
	setTimer(&myTimers[3]);
	setTimer(&myTimers[4]);
	setTimer(&myTimers[5]);
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

            injectChar(8, 3 + 2 * curTimer, animFrame[curTimer]++); //write the current frame character at the right spot
            if(animFrame[curTimer] > 14) animFrame[curTimer] = 1; //loop back to first anim frame
			myTimers[curTimer].absolute += delays[curTimer]; //prep the next timer
            setTimer(&myTimers[curTimer]); // send it to system
            }
        }
return 0;}
