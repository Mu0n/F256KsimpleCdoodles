/*
Code by Michael Juneau - January 2025 for the F256Jr, F256K, F256Jr2 and F256K2

simple timer0 example where we just watch the counter fill up from 0 to 0xFFFFFF,
let it trigger an interrupt and update a character color on screen. 

In order to achieve this, the last kernel block at E000 was copied into RAM and the MMU
LUT was changed to reflect this. This used some of Grenouye's code from Kooyan to achieve
this relocation. 
*/

#define F256LIB_IMPLEMENTATION

#include "f256lib.h"
#include "../src/muUtils.h"
#include "../src/muTimer0Int.h"

#define RATE 0x00FFFFFF

uint8_t color = 1; //used for text color for when we write ** on screen

void writeStars(void);


/**
 * Handle interrupts
 */
__attribute__((noinline)) __attribute__((interrupt_norecurse))
void irq_handler() {
    byte irq0 = PEEK(INT_PENDING_0);
    if ((irq0 & INT_TIMER_0) > 0) {
		byte LUT = PEEK(0);
		POKE(INT_PENDING_0, irq0 & 0xFE);
		POKE(0,0xB3);
		writeStars();
		loadTimer(RATE);
		POKE(0, LUT);
    }
    // Handle other interrupts as normal
    //original_irq_handler();
    //asm volatile("jmp (%[mem])" : : [mem] "r" (original_irq_handler));
}

void writeStars()
{
	color= color?0:1; //flip the color between 2 values
	textSetColor(color, 0); textGotoXY(0,1); textPrint("*");
}


int main(int argc, char *argv[]) {

original_irq_handler = (funcPtr)PEEKW(0xFFFE);

printf("this is supposed to trigger every 2/3rds of a second\n");

setTimer0(RATE);
asm("SEI");
loadTimer(RATE);

enableTimer(irq_handler, writeStars);

asm("CLI");

while(true)
	;
return 0;}
}