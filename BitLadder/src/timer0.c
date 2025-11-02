#include "f256lib.h"
#include "../src/timer0.h"


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
}