#include "D:\F256\llvm-mos\code\BitLadder\.builddir\trampoline.h"

//DEFINES
#define F256LIB_IMPLEMENTATION

#define ABS(a) ((a) < 0 ? -(a) : (a))

#define TIMER_SECONDS 1
#define TIMER_CAR_COOKIE 0
#define TIMER_LANE_COOKIE 1
#define TIMER_CAR_FORCE_COOKIE 2
#define TIMER_CAR_DELAY 1
#define TIMER_LANE_DELAY 3
#define TIMER_CAR_FORCE_DELAY 3

#define VKY_SP0_CTRL  0xD900 //Sprite #0’s control register
#define VKY_SP0_AD_L  0xD901 // Sprite #0’s pixel data address register
#define VKY_SP0_AD_M     0xD902
#define VKY_SP0_AD_H     0xD903
#define VKY_SP0_POS_X_L  0xD904 // Sprite #0’s X position register
#define VKY_SP0_POS_X_H  0xD905
#define VKY_SP0_POS_Y_L  0xD906 // Sprite #0’s Y position register
#define VKY_SP0_POS_Y_H  0xD907
#define SPR_CLUT_COLORS       32

#define PAL_BASE    0x10000
#define BITMAP_BASE      0x10400
 
#define TIMER_FRAMES 0

//INCLUDES
#include "f256lib.h"
#include <stdlib.h>
#include "../src/muUtils.h" //contains helper functions I often use
#include "../src/timer0.h"  //contains timer0 routines

EMBED(starspal, "../assets/stars.pal", 0x10000); //1kb
EMBED(starsbm, "../assets/stars.data",0x10400); //70kb

//GLOBALS
struct timer_t carTimer;

//GLOBALS


//FUNCTION PROTOTYPES
void setup(void);

void setup()
{
	uint32_t c;
	
	POKE(MMU_IO_CTRL, 0x00);
	// XXX GAMMA  SPRITE   TILE  | BITMAP  GRAPH  OVRLY  TEXT
	POKE(VKY_MSTR_CTRL_0, 0b00101111); //sprite,graph,overlay,text
	// XXX XXX  FON_SET FON_OVLY | MON_SLP DBL_Y  DBL_X  CLK_70
	POKE(VKY_MSTR_CTRL_1, 0b00000100); //font overlay, double height text, 320x240 at 60 Hz;
	POKE(VKY_LAYER_CTRL_0, 0b00000001); //bitmap 1 in layer 0, bitmap 0 in layer 1
	POKE(VKY_LAYER_CTRL_1, 0b00000010); //bitmap 2 in layer 2
	POKE(0xD00D,0x00); //force dark gray graphics background
	POKE(0xD00E,0x00);
	POKE(0xD00F,0x00);
	POKE(MMU_IO_CTRL,1);  //MMU I/O to page 1
	// Set up CLUT0.
	for(c=0;c<1023;c++)
	{
		POKE(VKY_GR_CLUT_0+c, FAR_PEEK(PAL_BASE+c));
	}
	POKE(MMU_IO_CTRL,0);
	
	bitmapSetActive(2);
	bitmapSetAddress(2,BITMAP_BASE);
	bitmapSetCLUT(0);
	
	bitmapSetVisible(0,false);
	bitmapSetVisible(1,false);
	bitmapSetVisible(2,true); //furthermost to act as a real background

}

	
int main(int argc, char *argv[]) {
	setup();
	hitspace();

	return 0;}
