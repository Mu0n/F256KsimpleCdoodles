/*
 * muTimer0Int.c: Timer 0 runs at the 25.175MHz dot clock and uses a 24 bit value
 * where 0x00FFFFFF is equal to 2/3rds of a second. For longer intervals, keep an extra
 * variable to do n loops of that maximum extent. This assumes you want to use it 
 * in the count up mode. Modify for your specific use case by consulting the different
 * modes in the .h file.
 * v1.0 July 21st 2026
 * Written by Mu0n aka 1Bit Fever Dreams aka AnyBits Fever Dreams
 *
 *
 * Typical usage:
 * 1) Assuming you have a delay in seconds with 6 decimal places granularity, i.e. 
 * delay = 0.192425s
 * convert to 24bit with: delay * 0x00FFFFFF / (2/3) and cut off the decimal part
 * 2) Use setTimer0(delay) to define the timer and immediately start it
 * 3) Check bit 4 of INT_PENDING_0 (0xD660) and if set, then the timer0 goal has been reached
 * 4) use resetTimer0 to start it again for periodic lengths of time, or setTimer0(newValue)
 * if you need a new delay

 * If your delay is higher than 0.666666 seconds:
 * use your own control loop to do multiple passes like so:
 *
// 
//this line is part of the setup of the timer
//

longDelay = 0x24FFFFFF; //setup: goes over the max of 0x00FFFFFF

//
// next lines are part of your control loops
//

if(longDelay > 0x00FFFFFF)
{
	setTimer0(0x00FFFFFF);
	longDelay -= 0x00FFFFFF;
}
else setTimer0(longDelay); //perform the last delay that falls under 0x00FFFFFF
 *
 *
 */

#ifndef MUTIMER0INT_C
#define MUTIMER0INT_C

#include "f256lib.h"
#include "../src/muTimer0Int.h"


bool globalPause = false;
bool tuneEnded = false;

void loadTimer0(uint32_t value) {
	POKE(T0_CTR, CTR_CLEAR);
    POKEA(T0_CMP_L, value);
}

void setTimer0(uint32_t value) {
	loadTimer0(value);//inject the compare value as max value
	resetTimer0();
}

void resetTimer0() {
	POKE(T0_CMP_CTR, 0); //when the target is reached, bring it back to value 0x000000
	POKE(T0_CTR, CTR_CLEAR);
	POKE(T0_CTR, CTR_INTEN | CTR_UPDOWN | CTR_ENABLE);
}


void loadTimer1(uint32_t value) {
	POKE(T1_CTR, CTR_CLEAR);
    POKEA(T1_CMP_L, value);
}

void setTimer1(uint32_t value) {
	loadTimer1(value);//inject the compare value as max value
	resetTimer1();
}

void resetTimer1() {
	POKE(T1_CMP_CTR, 0); //when the target is reached, bring it back to value 0x000000
	POKE(T1_CTR, CTR_CLEAR);
	POKE(T1_CTR, CTR_INTEN | CTR_UPDOWN | CTR_ENABLE);
}


#endif // MUTIMER0INT_C
