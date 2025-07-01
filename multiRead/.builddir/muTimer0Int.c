#include "c:\F256\f256llvm-mos\F256KsimpleCdoodles\multiRead\.builddir\trampoline.h"

#include "f256lib.h"
#include "../src/muTimer0Int.h"


funcPtr original_irq_handler;

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


/**
 * We'll have to find a way to handle the interrupts
 */
void enableTimer(funcPtr funcA, funcPtr funcB) {
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
    // copy the irq_handler function to B300
    uint16_t handler_addr = (uint16_t)funcA;
    uint16_t doSomething_addr = (uint16_t)funcB;
    uint16_t relocate = 0xB300;
    for (int i=handler_addr;i < doSomething_addr ;i++) {
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
