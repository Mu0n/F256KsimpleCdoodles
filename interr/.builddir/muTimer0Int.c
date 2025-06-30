#include "D:\F256\llvm-mos\code\interr\.builddir\trampoline.h"

#include "f256lib.h"
#include "../src/muTimer0Int.h"


/**
 * Set the MMU slot to a given bank.
 * Return the content of the previous bank.
 */
 //byte old_slot = setMMU(5, 0x3F);
byte setMMU(byte mmu_slot, byte bank) {
    // switch to edit mode
    byte mmu = PEEK(0);
    POKE(0,mmu | 0x80);
    byte old = PEEK(8+mmu_slot);
    POKE(8+mmu_slot, bank);
    // restore the mmu
    POKE(0, mmu);
    return old;
}


void loadTimer(uint32_t value) {
    POKEA(T0_CMP_L, value);
}


void setTimer0(uint32_t value)
{
	resetTimer0();
	loadTimer(value);//inject the compare value as max value
}

void resetTimer0(){
	POKE(T0_CMP_CTR, T0_CMP_CTR_RECLEAR); //when the target is reached, bring it back to value 0x000000
	POKE(T0_CTR, CTR_CLEAR);
	POKE(T0_CTR, CTR_UPDOWN | CTR_ENABLE);
}
