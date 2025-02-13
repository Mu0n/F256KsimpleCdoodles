#include "D:\F256\llvm-mos\code\lightshow\.builddir\trampoline.h"

#define F256LIB_IMPLEMENTATION

#include "f256lib.h"
#include "../src/muUtils.h" //contains helper functions I often use
#include "../src/muleds.h" //contains led related stuff

int main(int argc, char *argv[]) {
	uint16_t r1,r2,r3,r4,r5,r6;
	
	textGotoXY(1,20);
	if(isAnyK())
		{
		if(hasCaseLCD())
			{
			enableManualLEDs(true, true, true, true, true, true); //K2 is assumed here
			textPrint("a F256K2 is detected. 4 LEDs will participate.");
			}
		else
			{
			enableManualLEDs(false, true, true, true, true, true); //K is assumed here
			textPrint("a F256K is detected. 3 LEDs will participate.");
			}
		}
	else
	{
		enableManualLEDs(false, false, true, true, true, true); //Jr. or Jr.Jr. is assumed here
		textPrint("a F256Jr. or Jr2 is detected. 2 LEDs will participate.");
	}
	
	while(true)
	{
		r1 = randomRead();
		r2 = randomRead();
		r3 = randomRead();
		r4 = randomRead();
		r5 = randomRead();
		r6 = randomRead();
		POKE(LED_PWR_B, HIGH_BYTE(r1)); //power blue
		POKE(LED_PWR_G, LOW_BYTE(r1));  //power green
		POKE(LED_PWR_R, HIGH_BYTE(r2)); //power red
		POKE(LED_SD_B, LOW_BYTE(r2));  //media blue
		POKE(LED_SD_G, HIGH_BYTE(r3)); //media green
		POKE(LED_SD_R, LOW_BYTE(r3));  //media red
		POKE(LED_LCK_B, HIGH_BYTE(r4)); //lock blue
		POKE(LED_LCK_G, LOW_BYTE(r4));  //lock green
		POKE(LED_LCK_R, HIGH_BYTE(r5));  //lock red
		POKE(LED_NET_B, LOW_BYTE(r5)); //lock blue
		POKE(LED_NET_G, HIGH_BYTE(r6));  //lock green
		POKE(LED_NET_R, LOW_BYTE(r6));  //lock red
		lilpause(20);
	}
	
	while(true);
	return 0;}
