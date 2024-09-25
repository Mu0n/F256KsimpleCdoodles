#include "D:\F256\llvm-mos\code\write-on-screen\.builddir\trampoline.h"

#define F256LIB_IMPLEMENTATION
#include "f256lib.h"


int main(int argc, char *argv[]) {
	byte x, y;
	byte count=0;
	
	textClear();
	textEnableBackgroundColors(true);
	
	while(true)
	{
		textSetColor(LOW_BYTE(randomRead()),LOW_BYTE(randomRead()));
		textPrint("allo");
		textGetXY(&x,&y);
		if(y>39) textGotoXY(0,0);
		if(count++==2) {
			count=0;
		}
		else {
	
		graphicsWaitVerticalBlank();
		}
	}
	return 0;
}
