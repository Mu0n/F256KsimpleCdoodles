#include "D:\F256\llvm-mos\code\midiStuff\.builddir\trampoline.h"

#define F256LIB_IMPLEMENTATION

#define MIDI_CTRL 0xDDA0
#define MIDI_OUT 0xDDA1
#define MIDI_RX_00_07 0xDDA2
#define MIDI_RX_08_10 0xDDA3

#define TIMER_FRAMES 0
#define TIMER_SECONDS 1


#define TIMER_COOKIE 0
#define TIMER_REF_COOKIE 1
#define TIMER_DELAY 10
#define TIMER_REF_DELAY 1

#define SCREENCORNER 0xC000

#include "f256lib.h"
struct timer_t midiTimer, refTimer; //timer_t structure for setting timer through the kernel

bool setTimer(const struct timer_t *timer)
{
    *(uint8_t*)0xf3 = timer->units;
    *(uint8_t*)0xf4 = timer->absolute;
    *(uint8_t*)0xf5 = timer->cookie;
    kernelCall(Clock.SetTimer);
	return !kernelError;
}

void setup()
{
	
	textClear();
	textDefineForegroundColor(0,0xff,0xff,0xff);
    textGotoXY(0,0); textPrint("running a MIDI test");
	textGotoXY(0,2); textPrint("0xDDA0");
	textGotoXY(0,3); textPrint("0xDDA1");
	textGotoXY(0,4); textPrint("0xDDA2");
	textGotoXY(0,5); textPrint("0xDDA3");
	textGotoXY(0,6); textPrint("0xDDA4");
	textGotoXY(0,7); textPrint("0xDDA5");
	textGotoXY(0,8); textPrint("0xDDA6");
	textGotoXY(0,9); textPrint("0xDDA7");
	
	midiTimer.units = TIMER_FRAMES;
	midiTimer.absolute = TIMER_DELAY;
    midiTimer.cookie = TIMER_COOKIE;
	
	refTimer.units = TIMER_FRAMES;
	refTimer.absolute = TIMER_REF_DELAY;
	refTimer.cookie = TIMER_REF_COOKIE;
	
	setTimer(&midiTimer);
	setTimer(&refTimer);
	

}

void refreshPrints()
{
	textGotoXY(10,2); textPrintInt(PEEK(0xDDA0));
	textGotoXY(10,3); textPrintInt(PEEK(0xDDA1));
	textGotoXY(10,4); textPrintInt(PEEK(0xDDA2));
	textGotoXY(10,5); textPrintInt(PEEK(0xDDA3));
	textGotoXY(10,6); textPrintInt(PEEK(0xDDA4));
	textGotoXY(10,7); textPrintInt(PEEK(0xDDA5));
	textGotoXY(10,8); textPrintInt(PEEK(0xDDA6));
	textGotoXY(10,9); textPrintInt(PEEK(0xDDA7));
}
int main(int argc, char *argv[]) {
	uint8_t note;
	byte curTimer;
	
	setup();
	POKE(1,0);
    while(true)
        {
		kernelNextEvent();
        if(kernelEventData.type == kernelEvent(timer.EXPIRED))
            {
			curTimer = kernelEventData.timer.cookie; // find out which timer expired
			switch(curTimer)
			{
				case TIMER_COOKIE:
					POKE(MIDI_OUT, 0x80);
					POKE(MIDI_OUT, note);
					POKE(MIDI_OUT, 0x4F);
					midiTimer.absolute += TIMER_DELAY;
					setTimer(&midiTimer); // send it to system
	
					note = 0x20 + randomRead();
					POKE(MIDI_OUT, 0x90);
					POKE(MIDI_OUT, note);
					POKE(MIDI_OUT, 0x4F);
					break;
				case TIMER_REF_COOKIE:
					refreshPrints();
					refTimer.absolute += TIMER_REF_DELAY;
					setTimer(&refTimer); // send it to system
					break;
			}

            }
        }
return 0;}
