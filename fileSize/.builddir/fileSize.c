#include "D:\F256\llvm-mos\code\fileSize\.builddir\trampoline.h"

#define F256LIB_IMPLEMENTATION
#define FILE_COOKIE 21



#include "f256lib.h"
#include <stdio.h>
#include "../src/muUtils.h"

void backgroundSetup()
{
	POKE(MMU_IO_CTRL, 0x00);
	// XXX GAMMA  SPRITE   TILE  | BITMAP  GRAPH  OVRLY  TEXT
	POKE(VKY_MSTR_CTRL_0, 0b00101111); //sprite,graph,overlay,text
	// XXX XXX  FON_SET FON_OVLY | MON_SLP DBL_Y  DBL_X  CLK_70
	POKE(VKY_MSTR_CTRL_1, 0b00000100); //font overlay, double height text, 320x240 at 60 Hz;
	POKE(VKY_LAYER_CTRL_0, 0b00000001); //bitmap 1 in layer 0, bitmap 0 in layer 1
	POKE(VKY_LAYER_CTRL_1, 0b00000010); //bitmap 2 in layer 2
	POKE(0xD00D,0x00); //force black graphics background
	POKE(0xD00E,0x00);
	POKE(0xD00F,0x00);

	bitmapSetActive(0);
	bitmapSetCLUT(0);
	
	bitmapSetVisible(0,false);
	bitmapSetVisible(1,false);
	bitmapSetVisible(2,false);
	
}


#pragma push_macro("EOF")
#undef EOF

int main(int argc, char *argv[]) {
uint16_t fileID=0;
uint32_t sizeSoFar=0;

backgroundSetup();

kernelArgs->file.open.drive = 0;
kernelArgs->common.buf = "ronald.mp3";
kernelArgs->common.buflen = 11;
kernelArgs->file.open.mode = 0;
kernelArgs->file.open.cookie = FILE_COOKIE;

kernelCall(File.Open);

while(true)
{
	kernelNextEvent();
	if(kernelEventData.type == kernelEvent(file.OPENED))
		{
			if(kernelEventData.file.cookie == FILE_COOKIE) {
				fileID = kernelEventData.file.stream;
				break;
			}
		}
}

printf("\nThe file was opened! Now reading stream ID=%d...",fileID);

kernelArgs->file.read.stream = fileID;
kernelArgs->file.read.buflen = 255;

kernelCall(File.Read);
while(true)
{
	kernelNextEvent();
	if(kernelEventData.type == kernelEvent(file.DATA))
		{
			sizeSoFar+= kernelEventData.file.data.delivered;
	
			kernelArgs->file.read.stream = fileID;
			kernelArgs->file.read.buflen = 255;
			kernelCall(File.Read);
	
		}
	
	if(kernelEventData.type == kernelEvent(file.EOF))
		{
			sizeSoFar+= kernelEventData.file.data.delivered;
	
			break;
		}
	
}

printf("\n%lu bytes read",sizeSoFar);
printf("\nHit space to end.");
hitspace();
return 0;}

#pragma pop_macro("EOF")

