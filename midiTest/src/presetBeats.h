#ifndef PRESETBEATS_H
#define PRESETBEATS_H

#define TIMER_BEAT_1A 6 
#define TIMER_BEAT_1B 7

#include "f256lib.h"

//Preset Beats and tunes start at BASE_PRESETS in far memory (could be 0x10000) defined in presetBeats.c
//byte 0: chip used; byte 1: channel target of the chip; byte 2: instrument number; byte 3: noteCount; next n bytes: notes, next n bytes: delays
//then start over for next tracks of the beats

//this structure can contain small beats or small tunes, targeting multiple chips on multiple channels. some mallocs have to be applied before using, depending on complexity
typedef struct aB 
{
	bool isActive; //false= no
	bool pendingRelaunch; //for when you change tempo while it's playing. die down the beat first, then relaunch.
	uint8_t activeCount; //will increment when a note is sent, decrement when a note is died down
	uint8_t suggTempo; //suggested tempo it will switch to when switched on
	uint8_t howManyChans; //howMany is the 2nd dimension axis of the following arrays, ie if set as 2, you could keep a snare beat along with a bass track
	uint32_t baseAddr; //where in far memory the dynamically allocated data resides	
	
	uint8_t *index; //index is used during playback to keep track where it's at, per track
	uint8_t *pending2x; //for reach track, set true if a note has MSB set (0x80), meaning it has to double up in length before it really expires.
	//pending2x = 0 state it arrives at; 0x80 is detected. pass to 1, play note, don't die it down, pass to 2. when 2, go to next note.
	struct timer_t *timers; //timers to deal with these tracks' delays
} aBeat;

//this structure is used to inject data into far memory during a preset beat initinal load, and also 
//when the beat's playback is started, so that the instrument data can be set up
typedef struct aT
{
	uint8_t chip;
	uint8_t chan;
	uint8_t inst;
	uint8_t count;
} aTrack;

void setupBeats(struct aB *);
int8_t setupMem4ABeat(struct aB *, uint8_t, uint8_t, uint8_t, uint32_t *);
void setupMem4Track(struct aT, uint32_t *, const uint8_t *, const uint8_t *, uint8_t);
void getBeatTrackNoteInfo(struct aB *,uint8_t ,uint8_t ,uint8_t *,uint8_t *, struct aT *);
void punchInFar(uint8_t, uint32_t *);
void punchInFarArray(const uint8_t *, uint8_t, uint32_t *);
void beatSetInstruments(struct aT *);

extern const uint8_t presetBeatCount;
extern const char *presetBeatCount_names[];

#endif // PRESETBEATS_H