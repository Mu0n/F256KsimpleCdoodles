#include "D:\F256\llvm-mos\code\gui\.builddir\trampoline.h"

#include "f256lib.h"
#include "../src/mumouse.h"

void hideMouse()
{
	POKE(PS2_M_MODE_EN,0x00);
}

void showMouse()
{
	POKE(PS2_M_MODE_EN,0x01);
}
void prepMouse()
{
	showMouse(); //uses bit1= Mode 0 and bit0=enable the mouse
	POKEW(PS2_M_X_LO,0x100); //centers it in both x and y directions
	POKEW(PS2_M_Y_LO,0x100);
}
