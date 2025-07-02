#include "D:\F256\llvm-mos\code\multiRead\.builddir\trampoline.h"

#define F256LIB_IMPLEMENTATION

#include "f256lib.h"
#include "../src/muUtils.h" //contains helper functions I often use
#include "../src/muMidi.h" //contains helper functions I often use
#include "../src/muTimer0Int.h" //contains helper functions I often use
#include "../src/muMidiPlay2.h" //contains helper functions I often use

#define MIDISTART 0x10000

EMBED(music, "../doom.mid", 0x10000); //1kb


int main(int argc, char *argv[]) {
	
	midiShutAllChannels(false);
    initTrack(MIDISTART);

//find what to do and exhaust all zero delay events at the start
	exhaustZeroes();

	//insert game loop here
	while(true)
		{
		kernelNextEvent();
		if(PEEK(INT_PENDING_0)&0x10) //when the timer0 delay is up, go here
		{
			POKE(INT_PENDING_0,0x10); //clear the timer0 delay
			playMidi(); //play the next chunk of the midi file, might deal with multiple 0 delay stuff
		}
		sniffNextMIDI(); //find next event to play, will cue up a timer0 delay
		}
	destroyTrack();
	}
