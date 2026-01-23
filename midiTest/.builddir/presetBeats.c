#include "D:\F256\llvm-mos\code\miditest\.builddir\trampoline.h"

#include "f256lib.h"
#include "string.h"
#include "../src/presetBeats.h"
#include "../src/timerDefs.h"
#include "../src/musid.h"
#include "../src/muMidi.h"
#include "../src/muopl3.h"

#define BASE_PRESETS 0x20000

const uint8_t presetBeatCount = 5;
const char *presetBeatCount_names[] = {
	"WaveSynth        ",
	"Da Da Da         ",
	"Jazzy",
	"Funky",
	"M.U.L.E.         ",
//	"Ultima Underworld"
//	"Multi 001        ",
//	"Multi 002        ",
};



	//Beat 0, simple kick drum + snare beat
const uint8_t beat00_notes[] = {0x24,0x26};  //0x24 kick drum, 0x26 snare
const uint8_t beat00_delays[] = {3,3};
const struct aT beat00 = {.chip=0,.chan=9,.inst=0,.count=2};

	//Beat 1, famous casio beat used in Da Da Da
const uint8_t beat10_notes[] = {0x24,0x28,0x28,0x24,0x28};  //0x24 kick drum, 0x28 bright snare
const uint8_t beat10_delays[] = {3,2,2,3,3};
const uint8_t beat11_notes[] = {0x4B,0x63,0x00,0x63,0x4B,0x63,0x4B,0x63};
const uint8_t beat11_delays[] = {2,2,2,2,2,2,2,2};
const struct aT beat10 = {.chip=0,.chan=9,.inst=0   ,.count=5};
const struct aT beat11 = {.chip=0,.chan=2,.inst=0x73,.count=8};

	//Beat 2, jazz cymbal ride
static const uint8_t beat20_notes[] = {0x24,0x26,0x24,0x26};  //0x24 kick drum, 0x28 bright snare
static const uint8_t beat20_delays[] = {3,3,3,3};
static const uint8_t beat21_notes[] = {0x39,0x39,0x39,0x39,0x39,0x39};
static const uint8_t beat21_delays[] = {3,13,12,3,13,12};
static const struct aT beat20 = {.chip=0,.chan=9,.inst=0,.count=4};
static const struct aT beat21 = {.chip=0,.chan=9,.inst=0,.count=6};

	//Beat 3, funk swung 16th note
	
static const uint8_t beat30_notes[] = {0x2C,0x2C,0x2C,0x2C,0x2C,0x2C,0x2C,0x2C,0x2C,0x2C,0x2C,0x2C,0x2C,0x2C,0x2C,0x2C,
								0x2C,0x2C,0x2C,0x2C,0x2C,0x2C,0x2C,0x2C,0x2C,0x2C,0x2E,0x2E,0x2E,0x2E,0x2C,0x2C};
static const uint8_t beat30_delays[] = {16,15,16,15,16,15,16,15,16,15,16,15,16,15,16,15,16,15,16,15,16,15,16,15,16,15,16,15,16,15,16,15};
static const uint8_t beat31_notes[] = {0x24,0x00,0x24,0x26,0x00,0x24,0x00,0x24,0x24,0x26,0x00,0x24};
static const uint8_t beat31_delays[] = {17,16,15,17,16,15,16,15,17,17,16,15};
static const struct aT beat30 = {.chip=0,.chan=9,.inst=0,.count=32};
static const struct aT beat31 = {.chip=0,.chan=9,.inst=0,.count=12};

	//Beat 4, Mule bassline
const uint8_t beat40_notes[] =  {0,0,0,0,
								41,53,33,45,34,46,35,47,
								36,48,38,50,39,51,40,52,
								41,53,33,45,34,46,35,47,
								36,48,38,50,39,51,40,52,
								41,53,33,45,34,46,35,47,
								39,51,37,49,39,51,37,49,
								41,53,33,45,34,46,35,47,
								39,51,37,49,39,51,37,49,
								41,53,33,45,34,46,35,47,
								39,51,37,49,39,51,37,49,
								41,53,33,45,34,46,35,47,
								39,51,37,49,39,51,37,49,
								53,0,53,53,53,0};
const uint8_t beat40_delays[] = {5,5,5,5,
								 2,2,2,2,2,2,2,2,
								 2,2,2,2,2,2,2,2,
								 2,2,2,2,2,2,2,2,
								 2,2,2,2,2,2,2,2,
								 2,2,2,2,2,2,2,2,
								 2,2,2,2,2,2,2,2,
								 2,2,2,2,2,2,2,2,
								 2,2,2,2,2,2,2,2,
								 2,2,2,2,2,2,2,2,
								 2,2,2,2,2,2,2,2,
								 2,2,2,2,2,2,2,2,
								 2,2,2,2,2,2,2,2,
								 3,2,1,1,3,3};
const uint8_t beat41_notes[] = {23,35,47,35,23,35,47,35,
							    23,35,47,23,47,23,47,35}; //23 35 47
const uint8_t beat41_delays[] = {2,2,2,2,2,2,2,2,
							     2,2,2,2,2,2,2,2};
const struct aT beat40 = {.chip=1,.chan=0,.inst=1,.count=105};
const struct aT beat41 = {.chip=1,.chan=2,.inst=3,.count=16};

/*
	//Beat 5, Ultima Underworld "Descent"
static const uint8_t beat50_notes[] = {33,29,160 ,34,33,32,31,30,38,35,37,35};
static const uint8_t beat50_delays[]= {17,17,17,11 , 3, 3, 3,17,17,17,17,17};
const uint8_t beat51_notes[] = {53,56,60,65,68,72,53,56,60,65,68,72,53,56,60,65,68,72,
									   52,56,61,64,68,73,52,56,61,64,68,73,52,56,61,64,68,73,
									   52,56,59,64,68,71,52,56,59,64,68,71,52,56,59,64,68,71,
									   52,55,59,64,67,71,52,55,59,64,67,71,52,55,59,64,67,71,
									   53,56,60,65,68,72,53,56,60,65,68,72,53,56,60,65,68,72,
									   53,56,61,65,68,73,53,56,61,65,68,73,53,56,61,65,68,73,
									   53,58,62,65,70,74,53,58,62,65,70,74,53,58,62,65,70,74,
									   55,59,62,67,71,74,55,59,62,67,71,74,55,59,62,67,71,74,
									   53,58,62,65,70,74,53,58,62,65,70,74,53,58,62,65,70,74,
									   55,59,62,67,71,74,55,59,62,67,71,74,55,59,62,67,71,74,
									   53,58,62,65,70,74,53,58,62,65,70,74,53,58,62,65,70,74,
									   55,59,62,67,71,74,55,59,62,67,71,74,55,59,62,67,71,74};
const uint8_t beat51_delays[] = {1,1,1,1,1,1, 1,1,1,1,1,1, 1,1,1,1,1,1,
									    1,1,1,1,1,1, 1,1,1,1,1,1, 1,1,1,1,1,1,
									    1,1,1,1,1,1, 1,1,1,1,1,1, 1,1,1,1,1,1,
									    1,1,1,1,1,1, 1,1,1,1,1,1, 1,1,1,1,1,1,
									    1,1,1,1,1,1, 1,1,1,1,1,1, 1,1,1,1,1,1,
									    1,1,1,1,1,1, 1,1,1,1,1,1, 1,1,1,1,1,1,
									    1,1,1,1,1,1, 1,1,1,1,1,1, 1,1,1,1,1,1,
									    1,1,1,1,1,1, 1,1,1,1,1,1, 1,1,1,1,1,1,
									    1,1,1,1,1,1, 1,1,1,1,1,1, 1,1,1,1,1,1,
									    1,1,1,1,1,1, 1,1,1,1,1,1, 1,1,1,1,1,1,
									    1,1,1,1,1,1, 1,1,1,1,1,1, 1,1,1,1,1,1,
									    1,1,1,1,1,1, 1,1,1,1,1,1, 1,1,1,1,1,1};
static const struct aT beat50 = {.chip=3,.chan=2,.inst=20,.count=12};
const struct aT beat51 = {.chip=3,.chan=3,.inst=18, .count=216};


		//Beat 6, Multi chip 01
const uint8_t beat60_notes[] = {33,40,40,40,38,38,38,35,35,35,33,33};
const uint8_t beat60_delays[] = {10,4,4,10,4,4,10,4,4,10,4,4};
const uint8_t beat61_notes[] = {44,42,42,44,42,42,44,42};
const uint8_t beat61_delays[] = {9,3,3,9,3,9,3,3};
const uint8_t beat62_notes[] = {72,76,76,76,74,74,74,72,72,72,72,72};
const uint8_t beat62_delays[] = {10,4,4,10,4,4,10,4,4,10,4,4};
const uint8_t beat63_notes[] = {76,79,79,79,79,79,79,76,76,76,76,76};
const uint8_t beat63_delays[] = {10,4,4,10,4,4,10,4,4,10,4,4};
const uint8_t beat64_notes[] = {81,83,83,83,83,83,83,79,79,79,81,81};
const uint8_t beat64_delays[] = {10,4,4,10,4,4,10,4,4,10,4,4};
const uint8_t beat65_notes[] = {57,62, 59,64,67,64,59,64,59,64, 57,62,66,62,57,62,57,62, 55,60,64,60,55,60,55,60, 57,60,64,60,57,60};
const uint8_t beat65_delays[] = {9,9, 9,9,9,9,9,9,9,9, 9,9,9,9,9,9,9,9, 9,9,9,9,9,9,9,9, 9,9,9,9,9,9};
const struct aT beat60 = {.chip=3,.chan=1,.inst=12,.count=12};
const struct aT beat61 = {.chip=0,.chan=9,.inst=1,.count=8};
const struct aT beat62 = {.chip=0,.chan=2,.inst=10,.count=12};
const struct aT beat63 = {.chip=0,.chan=2,.inst=10,.count=12};
const struct aT beat64 = {.chip=0,.chan=2,.inst=10,.count=12};
const struct aT beat65 = {.chip=2,.chan=1,.inst=0,.count=32};
	
		//Beat 7, Multi chip 02
const uint8_t beat70_notes[] = {};
const uint8_t beat70_delays[] = {};
const struct aT beat70 = {.chip=3,.chan=1,.inst=12,.count=12};
*/
void beatSetInstruments(struct aT *theT)
{
	switch(theT->chip)
		{
		case 0: // MIDI
		    if(theT->chan != 0x09) {
				prgChange(theT->inst,theT->chan,true);
				prgChange(theT->inst,theT->chan,false);
				}
			break;
		case 1: // SID
			sid_setInstrument((theT->chan)/3, (theT->chan)-((theT->chan)/3)*3, sid_instrument_defs[theT->inst]);
			sid_setSIDWide(theT->inst);
			break;
		case 3: // OPL3
			opl3_setInstrument(opl3_instrument_defs[theT->inst],theT->chan);
			opl3_write(OPL_CH_FEED + theT->chan,   opl3_instrument_defs[theT->inst].CHAN_FEED);
			break;
		}
}

//setup memory for this beat according to how many tracks are used
int8_t setupMem4ABeat(struct aB *theB, uint8_t whichBeat, uint8_t tempo, uint8_t chanCountNeeded, uint32_t *whereAt)
{
	theB[whichBeat].isActive = false;
	theB[whichBeat].pendingRelaunch = false;
	theB[whichBeat].activeCount=0;
	theB[whichBeat].suggTempo = tempo;
	theB[whichBeat].howManyChans = chanCountNeeded;
	theB[whichBeat].baseAddr = *whereAt;
	
	theB[whichBeat].index = malloc(sizeof(uint8_t) * chanCountNeeded);
	if(theB[whichBeat].index == NULL) return -1;
	theB[whichBeat].pending2x = malloc(sizeof(uint8_t) * chanCountNeeded);
	if(theB[whichBeat].pending2x == NULL) return -1;
	theB[whichBeat].timers = malloc(sizeof(struct timer_t) * chanCountNeeded);
	if(theB[whichBeat].timers == NULL) return -1;
	
	for(uint8_t i = 0; i<chanCountNeeded; i++) {
		theB[whichBeat].index[i] = 0;
		theB[whichBeat].pending2x[i]=0;
		theB[whichBeat].timers[i].units = TIMER_FRAMES;
		theB[whichBeat].timers[i].cookie = TIMER_BEAT_1A + i;
		}
	return 0;
}

void punchInFar(uint8_t value, uint32_t *whereTo)
{
	FAR_POKE(*whereTo, value);
	(*whereTo)++;
}
void punchInFarArray(const uint8_t *theArray, uint8_t howMany, uint32_t *whereTo)
{
	for(uint8_t i=0; i < howMany; i++){
		FAR_POKE((*whereTo)++, theArray[i]);
	}
}

void getBeatTrackNoteInfo(struct aB *theB, uint8_t whichBeat, uint8_t track, uint8_t *farNote, uint8_t *farDelay, struct aT *theT)
{
	uint32_t addr = theB[whichBeat].baseAddr; //go to noteCount
	uint8_t farCount=0;
	
	for(uint8_t i = 0; i < theB[whichBeat].howManyChans; i++)
		{
	
		farCount = FAR_PEEK(addr+3);
		if(i==track){
			theT->chip = FAR_PEEK(addr);
			theT->chan = FAR_PEEK(addr+1);
			theT->inst = FAR_PEEK(addr+2);
			theT->count = FAR_PEEK(addr+3);
			*farNote  = FAR_PEEK(addr + 4 + (uint32_t)theB[whichBeat].index[track]);
			*farDelay = FAR_PEEK(addr + 4 + (uint32_t)(farCount) + (uint32_t)theB[whichBeat].index[track]);
			}
		else{
			addr += (uint32_t)(2*(farCount)+4);
			}
		}
}

void setupMem4Track(struct aT track, uint32_t *whereAt, const uint8_t *notes, const uint8_t *delays, uint8_t howMany)
{
	punchInFar(track.chip, whereAt);
	punchInFar(track.chan, whereAt);
	punchInFar(track.inst, whereAt);
	punchInFar(howMany, whereAt);
	punchInFarArray(notes, howMany, whereAt);
	punchInFarArray(delays, howMany, whereAt);
}

void setupBeats(struct aB *theBeats){
	uint32_t whereAt = BASE_PRESETS;
	
	//Beat 0, simple kick drum + snare beat
	if(setupMem4ABeat(theBeats, 0, 90, 1, &whereAt) == -1) printf("ERROR");
	setupMem4Track(beat00, &whereAt, beat00_notes, beat00_delays, sizeof(beat00_notes));
	
	//Beat 1, famous casio beat used in Da Da Da
	setupMem4ABeat(theBeats, 1, 128, 2, &whereAt);
	setupMem4Track(beat10, &whereAt, beat10_notes, beat10_delays, sizeof(beat10_notes));
	setupMem4Track(beat11, &whereAt, beat11_notes, beat11_delays, sizeof(beat11_notes));

	//Beat 2, jazz cymbal ride
	setupMem4ABeat(theBeats, 2, 100, 2, &whereAt);
	setupMem4Track(beat20, &whereAt, beat20_notes, beat20_delays, sizeof(beat20_notes));
	setupMem4Track(beat21, &whereAt, beat21_notes, beat21_delays, sizeof(beat21_notes));

	//Beat 3, funk swung 16th note
	setupMem4ABeat(theBeats, 3, 80, 2, &whereAt);
	setupMem4Track(beat30, &whereAt, beat30_notes, beat30_delays, sizeof(beat30_notes));
	setupMem4Track(beat31, &whereAt, beat31_notes, beat31_delays, sizeof(beat31_notes));
	
	//Beat 4, Mule bassline
	if(setupMem4ABeat(theBeats, 4, 120, 2, &whereAt) == -1) printf("ERROR");
	setupMem4Track(beat40, &whereAt, beat40_notes, beat40_delays, sizeof(beat40_notes));
	setupMem4Track(beat41, &whereAt, beat41_notes, beat41_delays, sizeof(beat41_notes));
/*
	//Beat 5, Ultima Underworld "Descent"
	if(setupMem4ABeat(theBeats, 3, 92, 1, &whereAt) == -1) printf("ERROR");
	setupMem4Track(beat50, &whereAt, beat50_notes, beat50_delays, sizeof(beat50_notes));
	setupMem4Track(beat51, &whereAt, beat51_notes, beat51_delays, sizeof(beat51_notes));

		//Beat 6, Multi 001"
	if(setupMem4ABeat(theBeats, 0, 120, 6, &whereAt) == -1) printf("ERROR");
	setupMem4Track(beat60, &whereAt, beat60_notes, beat60_delays, sizeof(beat60_notes));
	setupMem4Track(beat61, &whereAt, beat61_notes, beat61_delays, sizeof(beat61_notes));
	setupMem4Track(beat62, &whereAt, beat62_notes, beat62_delays, sizeof(beat62_notes));
	setupMem4Track(beat63, &whereAt, beat63_notes, beat63_delays, sizeof(beat63_notes));
	setupMem4Track(beat64, &whereAt, beat64_notes, beat64_delays, sizeof(beat64_notes));
	setupMem4Track(beat65, &whereAt, beat65_notes, beat65_delays, sizeof(beat65_notes));
	
		//Beat 7, Multi 002"
	if(setupMem4ABeat(theBeats, 1, 90, 1, &whereAt) == -1) printf("ERROR");
	setupMem4Track(beat00, &whereAt, beat00_notes, beat00_delays, sizeof(beat00_notes));
	*/
}
