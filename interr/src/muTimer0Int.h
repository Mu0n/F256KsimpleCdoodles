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

typedef void (*funcPtr)();

byte setMMU(byte, byte);
void enableTimer(funcPtr, funcPtr);
void loadTimer(uint32_t);
void setTimer0(uint32_t);
void resetTimer0(void);

extern funcPtr original_irq_handler;

#endif // MUTIMER0INT_H