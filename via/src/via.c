#define F256LIB_IMPLEMENTATION
#define VIA_BASE 0xDC00
#define VIA_T1CL (VIA_BASE + 0x04) // Timer 1 Counter Low
#define VIA_T1CH (VIA_BASE + 0x05) // Timer 1 Counter High
#define VIA_ACR (VIA_BASE + 0x0B)  // Auxiliary Control Register
#define VIA_IFR (VIA_BASE + 0x0D)  // Interrupt Flag Register
#define VIA_IER (VIA_BASE + 0x0E)  // Interrupt Enable Register


#include "f256lib.h"
void setup_timer0(void);
void __attribute__((interrupt)) timer0_isr(void);
void enable_interrupts(void);


void setup_timer0() {
    // Set the timer counter value
    *((volatile unsigned char*)VIA_T1CL) = 0xFF; // Low byte
    *((volatile unsigned char*)VIA_T1CH) = 0xFF; // High byte

    // Configure the timer in the ACR
    *((volatile unsigned char*)VIA_ACR) |= 0x40; // Continuous mode

    // Enable the timer interrupt in the IER
    *((volatile unsigned char*)VIA_IER) |= 0x82; // Enable Timer 1 interrupt
}

void __attribute__((interrupt)) timer0_isr() {
    // Clear the interrupt flag
    *((volatile unsigned char*)VIA_IFR) &= ~0x82;

    // Your callback function code here
	POKE(0xDDA1,0x90);
	POKE(0xDDA1,0x39);
	POKE(0xDDA1,0x3F);
}
void enable_interrupts() {
    asm("sei"); // Set the Interrupt Disable flag
}

	
int main(int argc, char *argv[]) {
	setup_timer0();
	enable_interrupts();
	
	while(true);	
	return 0;}
