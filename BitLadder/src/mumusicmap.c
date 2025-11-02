#include "f256lib.h"

//index of this array is which MIDI channel you want to map out
//high nibble: sub chip channel that you want to target; ignored with MIDI
//low nibble: chip: 0=sam, 1=SID, 2=PSG, 3=OPL3, 4=VS F=MUTED
uint8_t chipXChannel[16] = {0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F};


void resetMap(){
	for(uint8_t i=0;i<16;i++) chipXChannel[i] = 0;
}

}