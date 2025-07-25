#include "D:\F256\llvm-mos\code\f256amp\.builddir\trampoline.h"

#include "f256lib.h"
#include "../src/muVS1053b.h"
#include "../src/muUtils.h"
//The VS1053b is present on the Jr2 and K2.


//routine used to enable any plugin
//example 1: VS1053b's midi mode (rtmplugin);
//example 2: VS1053b's spectrum analyzer (saplugin)
// size can be determined by using something like sizeof(chosenPlugin)/sizeof(chosenPlugin[0]),
//    just select one of the plugin labels above and replace 'chosenPlugin'
void initVS1053Plugin(uint32_t base, uint16_t size) {
    uint16_t n;
    uint16_t addr, val, i=0;
	uint32_t parser = base;
  while (i<size) {
	

    //addr = plugin[i++];
    //n = plugin[i++];
	addr = FAR_PEEK(parser) | (FAR_PEEK(parser+1)<<8);
	parser+=2;i++;
	n = FAR_PEEK(parser) | (FAR_PEEK(parser+1)<<8);
	parser+=2;i++;
	
    if (n & 0x8000U) { /* RLE run, replicate n samples */
      n &= 0x7FFF;
      //val = plugin[i++];
      val = FAR_PEEK(parser) | (FAR_PEEK(parser+1)<<8);
	  parser+=2;i++;
	  while (n--) {
        //WriteVS10xxRegister(addr, val);
        POKEW(VS_SCI_ADDR,addr);
        POKEW(VS_SCI_DATA,val);
        POKE(VS_SCI_CTRL,CTRL_Start);
        POKE(VS_SCI_CTRL,0);
		while ((PEEK(VS_SCI_CTRL) & CTRL_Busy) == CTRL_Busy)
			;
      }
    } else {           /* Copy run, copy n samples */
      while (n--) {
        //val = plugin[i++];
	    val = FAR_PEEK(parser) | (FAR_PEEK(parser+1)<<8);
	    parser+=2;i++;
		//WriteVS10xxRegister(addr, val);
        POKEW(VS_SCI_ADDR,addr);
        POKEW(VS_SCI_DATA,val);
        POKE(VS_SCI_CTRL,CTRL_Start);
        POKE(VS_SCI_CTRL,0);
		while ((PEEK(VS_SCI_CTRL) & CTRL_Busy) == CTRL_Busy)
			;
      }
    }
  }
}

//perform this routine to boost the chip's clock. Pretty much necessary to do before real time midi mode or mp3 playback
void boostVSClock() {
//target the clock register
POKEW(VS_SCI_ADDR, VS_SCI_ADDR_CLOCKF);
//aim for 2.5X clock multiplier, no frills
POKEW(VS_SCI_DATA,0xe000);
//trigger the command
POKE(VS_SCI_CTRL,CTRL_Start);
POKE(VS_SCI_CTRL,0);
//check to see if it's done
	while (PEEK(VS_SCI_CTRL) & CTRL_Busy)
		;
}

void boostVSBass(void) {
//target the clock register
POKEW(VS_SCI_ADDR, VS_SCI_ADDR_BASS);
//aim for 2.5X clock multiplier, no frills
POKEW(VS_SCI_DATA,0x00F6);
//trigger the command
POKE(VS_SCI_CTRL,CTRL_Start);
POKE(VS_SCI_CTRL,0);
//check to see if it's done
	while (PEEK(VS_SCI_CTRL) & CTRL_Busy)
		;
}

uint16_t checkClock() {
POKEW(VS_SCI_ADDR, VS_SCI_ADDR_CLOCKF);
//trigger the command
POKE(VS_SCI_CTRL, CTRL_Start | CTRL_RWn);
POKE(VS_SCI_CTRL,0);
//check to see if it's done
	while (PEEK(VS_SCI_CTRL) & CTRL_Busy)
		;
return PEEKW(VS_SCI_DATA);
}

uint16_t getNbBands() {
	//Getting the number of bands (should be 14)
	//target the wram addr register
	POKEW(VS_SCI_ADDR, VS_SCI_ADDR_WRAMADDR);
	//target the number of used bands in 0x1802
	POKEW(VS_SCI_DATA, 0x1802);
	//trigger the command
	POKE(VS_SCI_CTRL, CTRL_Start);
	POKE(VS_SCI_CTRL,0);
	//check to see if it's done
		while (PEEK(VS_SCI_CTRL) & CTRL_Busy)
			;
	//target the wram register
	POKEW(VS_SCI_ADDR, VS_SCI_ADDR_WRAM);
	//trigger the read command
	POKE(VS_SCI_CTRL, CTRL_Start | CTRL_RWn);
	POKE(VS_SCI_CTRL,0);
	//check to see if it's done
		while (PEEK(VS_SCI_CTRL) & CTRL_Busy)
			;
	return PEEKW(VS_SCI_DATA);
}
//Assuming the spectrum analyzer plugin has been loaded beforehand
//this can read the results for each of the default 14 bands
void getCenterSAValues(uint16_t nbBands, uint16_t *values) {
uint16_t i = 0;


for(i=0; i<nbBands; i++) {
	POKEW(VS_SCI_ADDR, VS_SCI_ADDR_WRAMADDR);
	//target the wram with band data in 0x1814
	POKEW(VS_SCI_DATA, 0x1814+i);
	POKE(VS_SCI_CTRL, CTRL_Start);
	POKE(VS_SCI_CTRL,0);
	//check to see if it's done
	while (PEEK(VS_SCI_CTRL) & CTRL_Busy)
		;

	//target the wram register
	POKEW(VS_SCI_ADDR, VS_SCI_ADDR_WRAM);
	
	//trigger the read command
	POKE(VS_SCI_CTRL, CTRL_Start | CTRL_RWn);
	POKE(VS_SCI_CTRL,0);
	//check to see if it's done
	while (PEEK(VS_SCI_CTRL) & CTRL_Busy)
		;
	values[i] = (PEEKW(VS_SCI_DATA)); //only pick the values, discard the peaks
	}
}

//Enable the Spectrum Analyzer
void initBigPatch(uint32_t addr) {
//const uint16_t *ptr = bigpatch;
initVS1053Plugin(addr, 4667);
}
//Enable the Spectrum Analyzer
void initSpectrum(uint32_t addr) {
//const uint16_t *ptr = saplugin2;
initVS1053Plugin(addr, 1000);}

