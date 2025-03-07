#define F256LIB_IMPLEMENTATION

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
int main(int argc, char *argv[]) {
long size = 0L;
FILE *theFile=NULL;

    backgroundSetup();
	
	theFile = fopen("ronald.mp3","rb");
	if(theFile != NULL)
		{
		if(fseek(theFile,0L,SEEK_END) != 0)
			{
				size = ftell((FILE*)theFile);
				if(size != -1L)
					{
					printf("%ld bytes\n",size);
					if(fseek(theFile,0L,SEEK_SET) != 0)
						{
						if(fclose(theFile) == -1) printf("\n couldn't close the file");
						}
					else printf("\nCouldn't get back to the start of the file");
					}
				else printf("\nCouldn't get the file size");
			}
		else printf("\nCouldn't seek to the end of the file");
		}
	else printf("\nCouldn't open the file");

printf("\nHit space to end.");
hitspace();	
return 0;}
}