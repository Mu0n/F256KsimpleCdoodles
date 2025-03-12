#include "D:\F256\llvm-mos\code\NES\.builddir\trampoline.h"

#define F256LIB_IMPLEMENTATION
#include "f256lib.h"
#include "../src/mupads.h"

//DEFINES

int main(int argc, char *argv[]) {

uint8_t c=0; //index of char to print to show it's not frozen
POKE(MMU_IO_CTRL, 0x00);
	
while(true)
{
	textGotoXY(0,0);printf("%02x",c++);
	pollNES();
	padPollDelayUntilReady();

	//show what's up
	textGotoXY(10,5);printf("pad0 status %02x", PEEK(PAD0));
	textGotoXY(10,6);printf("pad1 status %02x", PEEK(PAD1));
	textGotoXY(10,7);printf("pad2 status %02x", PEEK(PAD2));
	textGotoXY(10,8);printf("pad3 status %02x", PEEK(PAD3));
	//clear the trig
}


return 0;}
