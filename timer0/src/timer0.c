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
bool noteOnOff = true;

void toggleMIDI()
{
	noteOnOff?POKE(MIDI,0x90):POKE(MIDI,0x80);POKE(MIDI,0x39);POKE(MIDI,0x4F); //alternates between note on and note off
	noteOnOff = !noteOnOff; //flip the note on/off
}

int main(int argc, char *argv[]) {
uint32_t check;


POKE(T0_CTR, CTR_CLEAR);
POKE(T0_CTR, CTR_ENABLE | CTR_UPDOWN);
POKE(T0_CMP_CTR, T0_CMP_CTR_RECLEAR);
POKE(T0_CMP_L,0xFF);POKE(T0_CMP_M,0xFF);POKE(T0_CMP_H,0xFF); //inject the compare value
while(true)
{
	while(true)
	{
		check = (uint32_t)((PEEK(T0_VAL_H)))<<16 | (uint32_t)((PEEK(T0_VAL_M)))<<8 | (uint32_t)((PEEK(T0_VAL_L)));
		printf("%08lx  ",check);
		if(check > 0x00FFFF) break;

	}
	printf("**********");
toggleMIDI();
POKE(T0_CTR, CTR_CLEAR);
POKE(T0_CTR, CTR_ENABLE | CTR_UPDOWN);
POKE(T0_CMP_L,0x0F);POKE(T0_CMP_M,0x00);POKE(T0_CMP_H,0x00); //inject the compare value
}

return 0;}
}