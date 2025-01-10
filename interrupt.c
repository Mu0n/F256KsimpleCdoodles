#define F256LIB_IMPLEMENTATION

#include "f256lib.h"

#define PEND_0 0xD660 // pending for interrupt group 0
#define MASK_0 0xD66C // mask for interrupt group 0. set bit for ignore

void disable_IRQ_handling() {
    asm("sei"); // Set the Interrupt Disable flag
}

void enable_IRQ_handling() {
    asm("cli"); // Set the Interrupt Disable flag
}
int main(int argc, char *argv[]) {
	disable_IRQ_handling();
	
	enable_IRQ_handling();
	
	
	while(true);	
	return 0;}
