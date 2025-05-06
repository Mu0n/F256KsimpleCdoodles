#include "f256lib.h"
#include "../src/mumouse.h"

void prepMouse()
{
	POKE(PS2_M_MODE_EN,0x01); //uses bit1= Mode 0 and bit0=enable the mouse 
	POKEW(PS2_M_X_LO,0x100); //centers it in both x and y directions
	POKEW(PS2_M_Y_LO,0x100);
}
