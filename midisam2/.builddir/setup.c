#include "D:\F256\llvm-mos\code\midisam2\.builddir\trampoline.h"

#include "f256lib.h"
#include "../src/setup.h"


void initProgress()
{/*
	uint8_t i=0;
	textSetColor(15,0);textGotoXY(15,5);textPrint("[");
	for(i=0;i<51;i++) textPrint("_");
	textPrint("]");*/
}

void updateProgress(uint8_t prog)
{/*
	uint8_t i=0;
	textSetColor(5,0);textGotoXY(16,5);
	for(i=0;i<prog;i++) __putchar(18);
*/
}
void wipeStatus()
{
	textGotoXY(0,25);textPrint("                                        ");
}
void setColors()
{
	uint8_t backup, i;
	backup = PEEK(MMU_IO_CTRL);
	POKE(MMU_IO_CTRL,0);
	POKE(0xD800, 0xFF);  //blue
	POKE(0xD801, 0xFF); //green
	POKE(0xD802, 0xFF); //red
	POKE(0xD803, 0);
	for(i=1;i<6;i++) //do yellows
	{
		POKE(0xD800+4*i,  0);              //blue
		POKE(0xD800+4*i+1, 0xCD + (i-1) * 10); //green
		POKE(0xD800+4*i+2, 0xCD + (i-1) * 10); //red
		POKE(0xD800+4*i+3, 0); 		     //unused alpha
	}
	for(i=6;i<11;i++) //do oranges
	{
		POKE(0xD800+4*i,  0);              //blue
		POKE(0xD800+4*i+1, (0xCD + (i-6) * 10)/2); //green
		POKE(0xD800+4*i+2, 0xCD + (i-6) * 10); //red
		POKE(0xD800+4*i+3, 0); 		     //unused alpha
	}
	for(i=11;i<16;i++) //do reds
	{
		POKE(0xD800+4*i,  0);              //blue
		POKE(0xD800+4*i+1,0); //green
		POKE(0xD800+4*i+2, 0xCD + (i-11) * 10); //red
		POKE(0xD800+4*i+3, 0); 		     //unused alpha
	}
	POKE(MMU_IO_CTRL, backup);
}
