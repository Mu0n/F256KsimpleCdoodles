#define F256LIB_IMPLEMENTATION
#include "f256lib.h"

#define TILESET_ADDR 0x10400
#define TILEMAP_ADDR 0x15000

EMBED(xaa, "../assets/platform_clut", 0x10000);
EMBED(xab, "../assets/platform_bm",   0x10400);

//sets the text mode, the organ MIDI instrument and the midi timer, sets up the bitmap for Bach+piano gfx
void setup()
{
	uint16_t c;
	
	
	POKE(MMU_IO_CTRL, 0x00);
	POKE(VKY_MSTR_CTRL_0, 0x3F); //graphics and tile enabled
	POKE(VKY_MSTR_CTRL_1, 0x14); //320x240 at 60 Hz; double height text
	POKE(VKY_LAYER_CTRL_0, 0x01); //bitmap 0 in layer 1, bitmap 1 in layer 0
	POKE(VKY_LAYER_CTRL_1, 0x04); //tile 0 in layer 2
	
	POKE(MMU_IO_CTRL,1);  //MMU I/O to page 1
	//prep to copy over the palette to the CLUT
	for(c=0;c<1023;c++) 
	{
		POKE(VKY_GR_CLUT_0+c, FAR_PEEK(0x10000+c));
	}
	
	//Bach Bitmap at layer 0
	POKE(MMU_IO_CTRL,0);
	
	bitmapSetVisible(0,false);
	bitmapSetVisible(1,false);
	bitmapSetVisible(2,false);
	bitmapSetCLUT(0);
	bitmapSetActive(1);

	textClear();
	textDefineForegroundColor(0,0xff,0xff,0xff);
	textEnableBackgroundColors(true);
	
	//tiles setting
	tileDefineTileMap(0,TILEMAP_ADDR,16,40,30);
	tileDefineTileSet(0,TILESET_ADDR, false);
	
	for(c=0;c<20*30;c++)
	{
		FAR_POKEW(TILEMAP_ADDR+(uint32_t)c*2,c&0x003F);
		
	}
	
	tileSetVisible(0,true);
	tileSetVisible(1,false);
	tileSetVisible(2,false);
	
}

int main(int argc, char *argv[]) {
	setup();
	
	POKE(MMU_IO_CTRL,0);
    while(true) 
        {
        }
return 0;}
