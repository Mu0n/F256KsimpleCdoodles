#define F256LIB_IMPLEMENTATION
#include "f256lib.h"

#include "../src/muUtils.h"

int main(int argc, char *argv[]) {
	uint32_t c =0;
	POKE(MMU_IO_CTRL,0);
    textGotoXY(5,10);
	textPrint("FOENIX MEMTEST: ");
	while(true) 
        {
		textGotoXY(21,10);printf("%04lx PERCENT",c);
		//lilpause(2);
		c++;
        }
return 0;}
