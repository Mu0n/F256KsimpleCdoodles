#include "D:\F256\llvm-mos\code\vstest\.builddir\trampoline.h"

#define F256LIB_IMPLEMENTATION



#include "../src/muUtils.h" //contains helper functions I often use
#include "../src/muMidi.h"  //contains basic MIDI functions I often use

int main(int argc, char *argv[]) {
	bool wv = true; //use true if you want the VS1053b, false for SAM2695
	
	//openAllCODEC(); //if the VS1053b is used, this might be necessary for some board revisions
	boostVSClock();
	initVS1053MIDI();  //if the VS1053b is used
	
	midiNoteOn(0,0x4b,0x3F,wv);
	hitspace();
	midiNoteOff(0,0x4b,0x3F,wv);
	midiNoteOn(0,0x4d,0x3F,wv);
	hitspace();
	midiNoteOff(0,0x4d,0x3F,wv);
	midiNoteOn(0,0x4f,0x3F,wv);
	hitspace();
	midiNoteOff(0,0x4f,0x3F,wv);
	midiNoteOn(0,0x50,0x3F,wv);
	hitspace();
	midiNoteOff(0,0x50,0x3F,wv);
	midiNoteOn(0,0x52,0x3F,wv);
	hitspace();
	midiNoteOff(0,0x52,0x3F,wv);
	
	return 0;
}
