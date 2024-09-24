#include "C:\F256\f256llvm-mos\code\textClock\.builddir\trampoline.h"

#define F256LIB_IMPLEMENTATION
#define TIMER_QUERY 128
#define TIMER_FRAMES 0
#define TIMER_SECONDS 1
#include "f256lib.h"

bool set_timer(const struct timer_t *timer){
    *(uint8_t*)0xf3 = timer->units;
    *(uint8_t*)0xf4 = timer->absolute;
    *(uint8_t*)0xf5 = timer->cookie;
    return kernelCall(Clock.SetTimer);
}
byte bcdToDec(byte bcd){
    uint16_t tens = (bcd >> 4) & 0xF;
    uint16_t units = bcd & 0xF;
    return LOW_BYTE(tens * 10 + units);
}

int main(int argc, char *argv[]) {
    struct timer_t time_data;
    byte oldsec=0;

	time_data.units = TIMER_SECONDS;
	time_data.absolute = 1;
	time_data.cookie = TIMER_QUERY;
	
    set_timer(&time_data);
	
    textClear();
    textSetDouble(true,true);
    while(true)
        {
        kernelCall(Clock.GetTime);
 
        if(timer_data.seconds != oldsec)
            {
            oldsec=timer_data.seconds;
            textGotoXY(10,10);
            textPrint("                         ");
            textGotoXY(10,10);
            printf("month %d day %d %d:%d:%d",bcdToDec(timer_data.month),
                                bcdToDec(timer_data.day),
                                bcdToDec(timer_data.hours),
                                bcdToDec(timer_data.minutes),
                                bcdToDec(timer_data.seconds));
            }
        }
