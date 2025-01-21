#include "D:\F256\llvm-mos\code\NES\.builddir\trampoline.h"

#define F256LIB_IMPLEMENTATION
#include "f256lib.h"

//DEFINES

#define NES_CTRL    0xD880

#define NES_CTRL_TRIG 0b10000000
#define NES_STAT_DONE 0b01000000
//D880 settings
//        7  6  5  4 |  3  2   1  0
// NES_TRIG XX XX XX | XX MODE XX NES_EN
#define NES_CTRL_MODE_NES  0b00000001
#define NES_CTRL_MODE_SNES 0b00000101

#define NES_STAT    0xD880
#define NES_PAD0    0xD884
#define NES_PAD0_A      0x7F
#define NES_PAD0_B      0xBF
#define NES_PAD0_SELECT 0xDF
#define NES_PAD0_START  0xEF
#define NES_PAD0_UP     0xF7
#define NES_PAD0_DOWN   0xFB
#define NES_PAD0_LEFT   0xFD
#define NES_PAD0_RIGHT  0xFE

#define NES_PAD1    0xD886
#define NES_PAD2    0xD887
#define NES_PAD3    0xD88A

int main(int argc, char *argv[]) {

uint8_t c=0; //index of char to print to show it's not frozen
	
//set NES_CTRL
POKE(NES_CTRL,NES_CTRL_MODE_SNES);
	
while(true)
{
	textGotoXY(0,0);printf("%02x",c++);
	//trigger a read
	POKE(NES_CTRL,NES_CTRL_TRIG);
	//wait until it's ready
	while((PEEK(NES_STAT) & NES_STAT_DONE) != NES_STAT_DONE)
		;

	//show what's up
	textGotoXY(10,5);printf("pad0 status %02x", PEEK(NES_PAD0));
	textGotoXY(10,6);printf("pad1 status %02x", PEEK(NES_PAD1));
	textGotoXY(10,7);printf("pad2 status %02x", PEEK(NES_PAD2));
	textGotoXY(10,8);printf("pad3 status %02x", PEEK(NES_PAD3));
	//clear the trig
	POKE(NES_STAT_DONE, NES_CTRL_MODE_SNES);
}


return 0;}
