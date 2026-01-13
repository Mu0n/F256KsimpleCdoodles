#define F256LIB_IMPLEMENTATION
#include "f256lib.h"
#include "../src/mupads.h"
#include "../src/muUtils.h"

int main(int argc, char *argv[]) {

uint8_t c=0; //index of char to print to show it's not frozen
POKE(MMU_IO_CTRL, 0x00);
	
while(true)
{
	textGotoXY(0,0);printf("%02x",c++); //show it's not frozen by cycling this on screen
	pollNES();
	//show what's up
	textGotoXY(10,4);textPrint("NES pads");
	textGotoXY(10,5);printf("pad0 status %02x", PEEK(PAD0));
	textGotoXY(10,6);printf("pad1 status %02x", PEEK(PAD1));
	textGotoXY(10,7);printf("pad2 status %02x", PEEK(PAD2));
	textGotoXY(10,8);printf("pad3 status %02x", PEEK(PAD3));
	
	
	pollSNES();
	textGotoXY(30,4);textPrint("SNES pads");
	textGotoXY(30,5);printf("pad0 status %02x", PEEK(PAD0));
	textGotoXY(30,6);printf("pad1 status %02x", PEEK(PAD1));
	textGotoXY(30,7);printf("pad2 status %02x", PEEK(PAD2));
	textGotoXY(30,8);printf("pad3 status %02x", PEEK(PAD3));
	
}

return 0;}
