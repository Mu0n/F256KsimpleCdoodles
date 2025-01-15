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

#define T0_PEND     0xD660
#define T0_MASK     0xD66C

#define T0_CTR      0xD650 //master control register for timer0, write.b0=ticks b1=reset b2=set to last value of VAL b3=set count up, clear count down
#define T0_STAT     0xD650 //master control register for timer0, read bit0 set = reached target val

#define CTR_INTEN   0x80  //present only for timer1? or timer0 as well?
#define CTR_ENABLE  0x01
#define CTR_CLEAR   0x02
#define CTR_LOAD    0x04
#define CTR_UPDOWN  0x08

#define T0_VAL_L    0xD651 //current 24 bit value of the timer
#define T0_VAL_M    0xD652
#define T0_VAL_H    0xD653

#define T0_CMP_CTR  0xD654 //b0: t0 returns 0 on reaching target. b1: CMP = last value written to T0_VAL
#define T0_CMP_L    0xD655 //24 bit target value for comparison
#define T0_CMP_M    0xD656
#define T0_CMP_H    0xD657

#define T0_CMP_CTR_RECLEAR 0x01
#define T0_CMP_CTR_RELOAD  0x02


#define MIDI 0xDDA1 //sam2695 to hear that timer0 is working

uint8_t color = 1; //used for text color for when we write ** on screen

void noteOnMIDI(void);
void writeStars(void);
void setTimer0(void);
void resetTimer0(void);
uint32_t readTimer0(void);
uint8_t isTimerDone(void);

void noteOnMIDI()
{
	POKE(MIDI,0x90);POKE(MIDI,0x39);POKE(MIDI,0x4F); //alternates between note on and note off
}

void writeStars()
{
	color= color?0:1; //flip the color between 2 values
	textSetColor(color+10,0);textPrint("**"); //do something visually for machines who can't deal MIDI
}

void setTimer0()
{
	resetTimer0();
	POKE(T0_CMP_CTR, T0_CMP_CTR_RECLEAR); //when the target is reached, bring it back to value 0x000000
	POKE(T0_CMP_L,0xFF);POKE(T0_CMP_M,0xFF);POKE(T0_CMP_H,0xFF); //inject the compare value as max value
}

void resetTimer0()
{
	POKE(T0_CTR, CTR_CLEAR);
	POKE(T0_CTR, CTR_UPDOWN | CTR_ENABLE);
	POKE(T0_PEND,0x10); //clear pending timer0 
}
uint32_t readTimer0()
{
	return (uint32_t)((PEEK(T0_VAL_H)))<<16 | (uint32_t)((PEEK(T0_VAL_M)))<<8 | (uint32_t)((PEEK(T0_VAL_L)));
}

uint8_t isTimerDone()
{
	return PEEK(T0_PEND)&0x10;
}


int main(int argc, char *argv[]) {

setTimer0();
while(true)
{
	while(isTimerDone()==0);
	
	noteOnMIDI();
	writeStars();
	POKE(T0_PEND,0x10); //clear timer0 at 0x10
}
return 0;}
}