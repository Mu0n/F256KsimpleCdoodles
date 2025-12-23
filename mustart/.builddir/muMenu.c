#include "D:\F256\llvm-mos\code\mustart\.builddir\trampoline.h"

#include "f256lib.h"
#include "../src/muMenu.h"

const char *menuItems[2] = {
"vgmplayer2.pgz",    "music/cozymidi.pgz"
};

uint8_t itemCount = 2;

void displayMenu(uint8_t x, uint8_t y)
{
	for(uint8_t i=0; i < itemCount; i++)
	{
	textGotoXY(x,y+i);
	printf("%s\n",menuItems[i]);
	}
}
