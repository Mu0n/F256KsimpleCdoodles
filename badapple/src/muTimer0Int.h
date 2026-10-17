/*
 * muTimer0Int.h: Timer 0 runs at the 25.175MHz dot clock and uses a 24 bit value
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

#ifndef MUTIMER0INT_H
#define MUTIMER0INT_H

#define T0_CTR      0xD650 //master control register for timer0, write.b0=ticks b1=reset b2=set to last value of VAL b3=set count up, clear count down
#define T0_STAT     0xD650 //master control register for timer0, read bit0 set = reached target val
#define T0_VAL_L    0xD651 //current 24 bit value of the timer
#define T0_VAL_M    0xD652
#define T0_VAL_H    0xD653

#define T0_CMP_CTR  0xD654 //b0: t0 returns 0 on reaching target. b1: CMP = last value written to T0_VAL
#define T0_CMP_L    0xD655 //24 bit target value for comparison
#define T0_CMP_M    0xD656
#define T0_CMP_H    0xD657

#define T1_CTR      0xD658 //master control register for timer0, write.b0=ticks b1=reset b2=set to last value of VAL b3=set count up, clear count down
#define T1_CMP_CTR  0xD65C //b0: t0 returns 0 on reaching target. b1: CMP = last value written to T0_VAL
#define T1_CMP_L    0xD65D //24 bit target value for comparison
#define T1_CMP_M    0xD65E
#define T1_CMP_H    0xD65F

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

void loadTimer0(uint32_t);
void setTimer0(uint32_t);
void resetTimer0(void);

void loadTimer1(uint32_t);
void setTimer1(uint32_t);
void resetTimer1(void);

extern bool globalPause;
extern bool tuneEnded;
#endif // MUTIMER0INT_H