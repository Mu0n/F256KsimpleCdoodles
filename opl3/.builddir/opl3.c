#include "D:\F256\llvm-mos\code\opl3\.builddir\trampoline.h"

#define F256LIB_IMPLEMENTATION

#include "f256lib.h"
#include "../src/muopl3.h"
#include "../src/muUtils.h"




void play_melody() {
    uint8_t channel = 0, i,j; // Channel 0
	for(j=0;j<8;j++)
		{
		for(i=0;i<12;i++)
			{
			opl3_note(channel, opl3_fnums[i], j, true);
			lilpause(2);
			opl3_note(channel, opl3_fnums[i], j, false);
			}
		}
	printf("-=- ALERT! -=-\n");
}

int main(int argc, char *argv[]) {
	
	opl3_initialize();
	
	while(true) play_melody();
	return 0;}
