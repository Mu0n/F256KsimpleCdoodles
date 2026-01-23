#include "D:\F256\llvm-mos\code\FusionDrive\.builddir\trampoline.h"

#include "f256lib.h"
#include "../src/muTimer0Int.h"
#include "../src/muUtils.h"


void loadTimer(uint32_t value) {
	POKE(T0_CTR, CTR_CLEAR);
    POKEA(T0_CMP_L, value);
}

void setTimer0(uint32_t value)
{
	loadTimer(value);//inject the compare value as max value
	resetTimer0();
}

void resetTimer0(){
	POKE(T0_CMP_CTR, 0); //when the target is reached, bring it back to value 0x000000
	POKE(T0_CTR, CTR_CLEAR);
	POKE(T0_CTR, CTR_INTEN | CTR_UPDOWN | CTR_ENABLE);
}

void setSOL()
{
	POKE(LINT_CTRL,1); //enable start of line interrupt
	POKE(INT_MASK_0, PEEK(INT_MASK_0) & 0xFD); //clear bit 0x02 VKY_SOL
}
void setSOL_line(uint16_t line)
{
	POKEW(LINT_L_LO, line);
}
