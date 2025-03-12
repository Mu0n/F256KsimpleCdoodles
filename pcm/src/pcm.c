#define F256LIB_IMPLEMENTATION



#include "../src/muUtils.h" //contains helper functions I often use
#include "../src/muVS1053b.h"  //contains basic MIDI functions I often use

EMBED(dash, "../assets/dash.wav", 0x10000);

int main(int argc, char *argv[]) {
	uint16_t i=0;
	//openAllCODEC(); //if the VS1053b is used, this might be necessary for some board revisions	
	boostVSClock();
	initBigPatch();
	
	for(i=0;i<0xADDC;i++) POKE(VS_FIFO_DATA,FAR_PEEK(0x10000+i));
	
	return 0;
}	
}