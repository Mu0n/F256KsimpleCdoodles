#ifndef PRESETBEATS_H
#define PRESETBEATS_H

#define TIMER_BEAT_0 5 //wave synth beat
#define TIMER_BEAT_1A 6 //da da da beat 
#define TIMER_BEAT_1B 7
#define TIMER_BEAT_2A 8 //jazzy beat
#define TIMER_BEAT_2B 9
#define TIMER_BEAT_3A 10  //funky beat
#define TIMER_BEAT_3B 11

#include "f256lib.h"

typedef struct aB 
{
	bool isActive; //false= no
	bool pendingRelaunch; //for when you change tempo while it's playing. die down the beat first, then relaunch.
	uint8_t activeCount; //will increment when a note is sent, decrement when a note is died down
	uint8_t suggTempo; //suggested tempo it will switch to when switched on
	uint8_t howMany; //howMany is the 2nd dimension axis of the following arrays, ie if set as 2, you could keep a snare beat along with a bass track
	uint8_t *channel;//channel is used to indicate which of the 16 MIDI channel is used for this beat
	uint8_t *index; //index is used during playback to keep track where it's at, per track
	uint8_t **notes; //contains the midi notes to be sent out
	uint8_t *noteCount; //lets the loop be easy to detect
	uint8_t *instruments; //contains the 0xC? parameter for program change, ie setting the instrument; every track gets one
	uint8_t **delays; //contains the delays to wait until the next note
	struct timer_t *timers; //timers to deal with these tracks' delays
} aBeat;

void setupBeats(struct aB*);


#endif // PRESETBEATS_H