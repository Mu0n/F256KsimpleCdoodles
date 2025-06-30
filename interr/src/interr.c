/*
Code by Michael Juneau - January 2025 for the F256Jr, F256K, F256Jr2 and F256K2

simple timer0 example where we just watch the counter fill up from 0 to 0xFFFFFF,
observe its value in a dedicated loop and react out of it when the timer0 pending bit
is set in the T0_PEND register

In this current iteration, the kernel interrupts are completely disabled with asm("sei") to
avoid an occasional glitch (that happens roughly every 50ish give or take 20, loop iteration

Another more advanced usage of timer0 not done here is to let it trigger interrupts, react to those
by having edited the irq vector to handle them. 
*/

#define F256LIB_IMPLEMENTATION

#include "f256lib.h"
#include "../src/muUtils.h"
#include "../src/muTimer0Int.h"

#define RATE 0x00AFDEAD
#define T0_CTR      0xD650 //master control register for timer0, write.b0=ticks b1=reset b2=set to last value of VAL b3=set count up, clear count down
#define T0_STAT     0xD650 //master control register for timer0, read bit0 set = reached target val
#define T0_VAL_L    0xD651 //current 24 bit value of the timer
#define T0_VAL_M    0xD652
#define T0_VAL_H    0xD653

#define T0_CMP_CTR  0xD654 //b0: t0 returns 0 on reaching target. b1: CMP = last value written to T0_VAL
#define T0_CMP_L    0xD655 //24 bit target value for comparison
#define T0_CMP_M    0xD656
#define T0_CMP_H    0xD657

#define INT_PENDING_0  0xD660
#define INT_POLARITY_0 0xD664
#define INT_EDGE_0     0xD668
#define INT_MASK_0     0xD66C

#define INT_PENDING_1  0xD661
#define INT_POLARITY_1 0xD665
#define INT_EDGE_1     0xD669
#define INT_MASK_1     0xD66D

#define INT_PENDING_2  0xD662
#define INT_POLARITY_2 0xD666
#define INT_EDGE_2     0xD66A
#define INT_MASK_2     0xD66E

#define INT_VKY_SOF    0x01
#define INT_VKY_SOL    0x02
#define INT_PS2_KBD    0x04
#define INT_PS2_MOUSE  0x08
#define INT_TIMER_0    0x10
#define INT_TIMER_1    0x20
#define INT_CART       0x80


#define CTR_INTEN   0x80  //present only for timer1? or timer0 as well?
#define CTR_ENABLE  0x01
#define CTR_CLEAR   0x02
#define CTR_LOAD    0x04
#define CTR_UPDOWN  0x08

#define T0_CMP_CTR_RECLEAR 0x01
#define T0_CMP_CTR_RELOAD  0x02



#define MIDI 0xDDA1 //sam2695 to hear that timer0 is working

uint8_t color = 1; //used for text color for when we write ** on screen


void writeStars(void);
void enableTimer(void);

funcPtr original_irq_handler;


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

/**
 * We'll have to find a way to handle the interrupts
 */
void enableTimer() {
    original_irq_handler = (funcPtr)PEEKW(0xFFFE);
    //original_irq_handler = PEEKW(0xFFFE);
    // copy the $7F bank into another area of RAM - I guess DMA is not usable??
	
	//printf("before MMU");
	//hitspace();
    byte old_slot = setMMU(5, 0x3F);
    //printf("after MMU");
	//hitspace();
	
	for (int i = 0; i < 0x1FFE; i++) {
        byte flash = PEEK(0xE000 + i);
        POKE(0xA000 + i, flash);
    }
    // copy the irq_handler function to F300
    uint16_t handler_addr = (uint16_t)&irq_handler;
    uint16_t playvgm_addr = (uint16_t)&writeStars;
    uint16_t relocate = 0xB300;
    for (int i=handler_addr;i < playvgm_addr ;i++) {
        byte hdlr = PEEK(i);
        POKE(relocate++, hdlr);
    }
    // set the interrupt handler now
    POKEW(0xA000 + 0x1FFE, 0xF300);
    // replace the RTI for RTS
    POKE(0xA131,0x60);
    setMMU(5, old_slot);
    
    byte IRQ0 = PEEK(INT_MASK_0);
    // enable timer0 - bit 4
    IRQ0 &= 0xEF;
    POKE(INT_MASK_0, IRQ0);
    // Switch to the copy
    byte LUT = PEEK(0);
    POKE(0, (LUT & 0xF) + 0x80);
    POKE(0xF, 0x3F);
    POKE(0, (LUT & 0xF) + 0x90);
    POKE(0xF, 0x3F);
    POKE(0, (LUT & 0xF) + 0xA0);
    POKE(0xF, 0x3F);
    POKE(0, (LUT & 0xF) + 0xB0);
    POKE(0xF, 0x3F);
    // restore the LUT
    POKE(0, LUT);

    resetTimer0();
}


int main(int argc, char *argv[]) {

original_irq_handler = (funcPtr)PEEKW(0xFFFE);

printf("this is supposed to trigger every 2/3rds of a second\n");

setTimer0(RATE);
asm("SEI");
loadTimer(RATE);

enableTimer();

asm("CLI");

while(true)
	;
return 0;}
}