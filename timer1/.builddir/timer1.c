#include "D:\F256\llvm-mos\code\timer1\.builddir\trampoline.h"

/*
Code by Michael Juneau - January 2026 for the F256Jr, F256K, F256Jr2 and F256K2

simple timer1 example where we just watch the counter fill up from 0 to 0xFFFFFF,
observe its value in a dedicated loop and react out of it when the timer1 pending bit
is set in the T1_PEND register

*/

#define F256LIB_IMPLEMENTATION

#include "f256lib.h"

#define T1_CTR      0xD658 //master control register for timer0, write.b0=ticks b1=reset b2=set to last value of VAL b3=set count up, clear count down
#define T1_CMP_CTR  0xD65C //b0: t0 returns 0 on reaching target. b1: CMP = last value written to T0_VAL
#define T1_CMP_L    0xD65D //24 bit target value for comparison
#define T1_CMP_M    0xD65E
#define T1_CMP_H    0xD65F
#define INT_PENDING_0  0xD660

#define CTR_INTEN   0x80  //present only for timer1? or timer0 as well?
#define CTR_ENABLE  0x01
#define CTR_CLEAR   0x02
#define CTR_LOAD    0x04
#define CTR_UPDOWN  0x08



#define MIDI 0xDDA1 //sam2695 to hear that timer0 is working

uint8_t color = 1; //used for text color for when we write ** on screen

void noteOnMIDI(void);
void writeStars(void);
void setTimer1(uint32_t);
void loadTimer1(uint32_t);
void resetTimer1(void);

void noteOnMIDI()
{
	POKE(MIDI,0x90);POKE(MIDI,0x39);POKE(MIDI,0x4F); //alternates between note on and note off
}

void writeStars()
{
	color= color?0:1; //flip the color between 2 values
	textSetColor(color+10,0);textPrint("**"); //do something visually for machines who can't deal MIDI
}


void loadTimer1(uint32_t value) {
    POKE(T1_CTR, CTR_CLEAR);
    POKEA(T1_CMP_L, value);
}

void resetTimer1(){
    POKE(T1_CMP_CTR, 0); //when the target is reached, bring it back to value 0x000000
    POKE(T1_CTR, CTR_CLEAR);
    POKE(T1_CTR, CTR_INTEN | CTR_UPDOWN | CTR_ENABLE);
}

void setTimer1(uint32_t value)
{
    loadTimer1(value);//inject the compare value as max value
    resetTimer1();
}
int main(int argc, char *argv[]) {
	
	setTimer1(0x0000003F);
while(true)
{
	if(PEEK(INT_PENDING_0)&0x20) //timer1 interrupt is raised
		{
		noteOnMIDI();
		writeStars();
		POKE(INT_PENDING_0,0x20); //clear pending timer0
  //do stuff at the end of the delay
		setTimer1(0x0000003F); //set a 24-bit value from 0 to 0x00FFFFFF. Real world time is from 0s to 0.6666s. if you need more, set regular companion variables like a uint8_t that can cycle through 255 of these 2/3rds of a second. you get the idea.
  //and resend it once more, set the argument to taste
		}
	
}
return 0;}
