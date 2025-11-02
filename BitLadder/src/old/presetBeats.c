#include "f256lib.h"
#include "string.h"
#include "../src/presetBeats.h"
#include "../src/timerDefs.h"
#include "../src/musid.h"
#include "../src/muMidi.h"
#include "../src/muopl3.h"

#define BASE_PRESETS 0x40000

const uint8_t presetBeatCount = 5;
const char *presetBeatCount_names[] = {
	"Level 001 PSG ",
	"Level 001 SID ",
	"Level 001 OPL ",
	"Level 001 MID ",
	"Level 002 PSG "
};
	//Beat 0, simple kick drum + snare beat
const uint8_t beat00_notes[] = {0x24,0x26};  //0x24 kick drum, 0x26 snare
const uint8_t beat00_delays[] = {3,3};  

/*

uint8_t tempoLUTRef[30] = {4, 8, 16, 32, 64, 128, //         32nd, 16th, 8th, 4th, half, whole
						   6,12, 24, 48, 96, 192, //dotted   32nd, 16th, 8th, 4th, half, whole
					      11,21, 32,  5, 11,  144,
						  0,0,0,0,0,0,
						  0,0,0,0,0,0}; //triplets of 4th: lengths of 1, 2, 3,  triplets of 8ths of length 1, 2, 3
			
			
*/
// Level 002
const uint8_t beat20_notes[] = {28, 28, 28, 28, 31, 28, 28, 28, 28, 26,
							    28, 28, 28, 28, 31, 28, 28, 28, 28, 26,
								28, 28, 40, 28, 28, 28, 40, 28,
								28, 28, 40, 28, 28, 28, 40, 28,
								28, 28, 40, 28, 28, 28, 40, 28,
								28, 28, 40, 28, 28, 28, 40, 28,
								28, 28, 43, 36, 35, 35, 36, 38,
								28, 28, 43, 36, 35, 35, 36, 38,
								28, 28, 43, 36, 35, 35, 36, 38,
								28, 28, 43, 36, 35, 35, 36, 38,
								28, 35, 28, 31, 33, 28, 35, 28, 31, 30,
								28, 35, 28, 31, 33, 28, 35, 28, 31, 30};
const uint8_t beat20_delays[]= {14, 14,  2,  3,  3, 14,  14,  2,  3,  3,
							    14, 14,  2,  3,  3, 14,  14,  2,  3,  3,
								14,  1,  2, 14,  2,  1,  2,  2,
								14,  1,  2, 14,  2,  1,  2,  2,
								14,  1,  2, 14,  2,  1,  2,  2,
								14,  1,  2, 14,  2,  1,  2,  2,
								14,  1,  2, 14,  2,  1,  2,  2,
								14,  1,  2, 14,  2,  1,  2,  2,
								14,  1,  2, 14,  2,  1,  2,  2,
								14,  1,  2, 14,  2,  1,  2,  2,
								14, 14,  2,  3,  3, 14, 14,  2,  3,  3,
								14, 14,  2,  3,  3, 14, 14,  2,  3,  3};

const uint8_t beat21_notes[] = {59, 0, 57, 0, 55, 0, 54, 0};
const uint8_t beat21_delays[]= { 4, 4,  4, 4,  4, 4,  4, 4};
const uint8_t beat22_notes[] = {52, 0, 50, 0, 48, 0, 47, 0};
const uint8_t beat23_notes[] = {28, 52,  0, 28, 52, 0,
								28, 52,  0, 28, 52, 0,
								28, 28, 52, 0, 28, 28, 52, 28,
								28, 28, 52, 0, 28, 28, 52, 28,
								28, 28, 52, 0, 28, 28, 52, 28,
								28, 28, 52, 0, 28, 28, 52, 28,
								28, 28, 52, 0, 28, 28, 52, 28,
								28, 28, 52, 0, 28, 28, 52, 28,
								28, 28, 52, 0, 28, 28, 52, 28,
								28, 28, 52, 0, 28, 28, 52, 28,
								28, 52,  0, 28, 52, 0,
								28, 52,  0, 28, 52, 0};
const uint8_t beat23_delays[]= { 1,  16,  14,  1,  16, 14,
							     1,  16,  14,  1,  16, 14,
								 2, 2, 2, 2, 2, 2, 2, 2, 
								 2, 2, 2, 2, 2, 2, 2, 2, 
								 2, 2, 2, 2, 2, 2, 2, 2, 
								 2, 2, 2, 2, 2, 2, 2, 2, 
								 2, 2, 2, 2, 2, 2, 2, 2, 
								 2, 2, 2, 2, 2, 2, 2, 2, 
								 2, 2, 2, 2, 2, 2, 2, 2, 
								 1,  16,  14,  1,  16, 14,
							     1,  16,  14,  1,  16, 14};
	
const struct aT beat20 = {.chip=1,.chan=0,.inst=1,.count=104};				  
const struct aT beat21 = {.chip=1,.chan=1,.inst=0,.count=8};
const struct aT beat22 = {.chip=1,.chan=3,.inst=0,.count=6};
const struct aT beat23 = {.chip=1,.chan=4,.inst=5,.count=88};
		//Level 001 PSG
const uint8_t beat60_notes[] = {65,69,72,77, 72,69,72,69,
								65,69,72,75, 72,69,72,69,
								65,70,74,79, 74,70,74,70,
								65,71,74,77, 74,71,74,71};
const uint8_t beat60_delays[] = {3, 3, 3, 3,  3, 3, 9,21,
								 3, 3, 3, 3,  3, 3, 9,21,
								 3, 3, 3, 3,  3, 3, 9,21,
								 3, 3, 3, 3,  3, 3, 9,21};  
const struct aT beat60 = {.chip=2,.chan=0,.inst=0,.count=24};
	
		//Level 001 SID
const uint8_t beat71_notes[] = {36,48,36,48,36,48,48,36};
const uint8_t beat71_delays[] = {3, 3, 3, 3,  3, 3, 9,21}; 
const uint8_t beat72_notes[] = {41,41,41,41,53,41,53,41,
							    41,41,41,41,53,41,53,41,
								39,39,39,39,51,39,51,39,
								39,39,39,39,51,39,51,39,
								34,34,34,34,58,34,58,34,
								34,34,34,34,58,34,58,34,
								37,37,37,37,49,37,49,37,
							    37,37,37,37,37,36,39,40};
const uint8_t beat72_delays[] = {2,2,2,2, 2,2,2,2,
								 2,2,2,2, 2,2,2,2,
								 2,2,2,2, 2,2,2,2,
								 2,2,2,2, 2,2,2,2,
								 2,2,2,2, 2,2,2,2,
								 2,2,2,2, 2,2,2,2,
								 2,2,2,2, 2,2,2,2,
								 2,2,2,2, 2,2,2,2};   
const struct aT beat70 = {.chip=1,.chan=0,.inst=0,.count=24};
const struct aT beat71 = {.chip=1,.chan=1,.inst=5,.count=8};
const struct aT beat72 = {.chip=1,.chan=3,.inst=2,.count=48};

		//Level 001 OPL3
const uint8_t beat83_notes[] = {89,96,89, 87,96,87, 86,94,86, 85,82,87,88};
const uint8_t beat83_delays[] = {5, 4, 4,  5, 4, 4,  5, 4, 4,  5, 4, 3, 3}; 
const struct aT beat80 = {.chip=3,.chan=0,.inst=4,.count=24};
const struct aT beat81 = {.chip=3,.chan=1,.inst=13,.count=8};
const struct aT beat82 = {.chip=3,.chan=2,.inst=10,.count=48};
const struct aT beat83 = {.chip=3,.chan=3,.inst=16,.count=13};

		//Level 001 MIDI
const struct aT beat90 = {.chip=0,.chan=0,.inst=90,.count=24};
const struct aT beat91 = {.chip=0,.chan=1,.inst=113,.count=8};
const struct aT beat92 = {.chip=0,.chan=2,.inst=37,.count=48};
const struct aT beat93 = {.chip=0,.chan=3,.inst=91,.count=13};
const struct aT beat00 = {.chip=0,.chan=9,.inst=0,.count=2};

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
		theB[whichBeat].timers[i].cookie = TIMER_BEAT + i;
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
	
		//Level 001
	if(setupMem4ABeat(theBeats, 0, 140, 1, &whereAt) == -1) printf("ERROR");
	setupMem4Track(beat60, &whereAt, beat60_notes, beat60_delays, sizeof(beat60_notes));
	
	if(setupMem4ABeat(theBeats, 1, 140, 3, &whereAt) == -1) printf("ERROR");
	setupMem4Track(beat70, &whereAt, beat60_notes, beat60_delays, sizeof(beat60_notes));
	setupMem4Track(beat71, &whereAt, beat71_notes, beat71_delays, sizeof(beat71_notes));
	setupMem4Track(beat72, &whereAt, beat72_notes, beat72_delays, sizeof(beat72_notes));

	if(setupMem4ABeat(theBeats, 2, 140, 4, &whereAt) == -1) printf("ERROR");
	setupMem4Track(beat80, &whereAt, beat60_notes, beat60_delays, sizeof(beat60_notes));
	setupMem4Track(beat81, &whereAt, beat71_notes, beat71_delays, sizeof(beat71_notes));
	setupMem4Track(beat82, &whereAt, beat72_notes, beat72_delays, sizeof(beat72_notes));
	setupMem4Track(beat83, &whereAt, beat83_notes, beat83_delays, sizeof(beat83_notes));
		
	if(setupMem4ABeat(theBeats, 3, 140, 5, &whereAt) == -1) printf("ERROR");
	setupMem4Track(beat90, &whereAt, beat60_notes, beat60_delays, sizeof(beat60_notes));
	setupMem4Track(beat91, &whereAt, beat71_notes, beat71_delays, sizeof(beat71_notes));
	setupMem4Track(beat92, &whereAt, beat72_notes, beat72_delays, sizeof(beat72_notes));
	setupMem4Track(beat93, &whereAt, beat83_notes, beat83_delays, sizeof(beat83_notes));
	setupMem4Track(beat00, &whereAt, beat00_notes, beat00_delays, sizeof(beat00_notes));

		//Level 002
	if(setupMem4ABeat(theBeats, 4, 100, 3, &whereAt) == -1) printf("ERROR");
	setupMem4Track(beat20, &whereAt, beat20_notes, beat20_delays, sizeof(beat20_notes));
	setupMem4Track(beat21, &whereAt, beat21_notes, beat21_delays, sizeof(beat21_notes));
	setupMem4Track(beat22, &whereAt, beat22_notes, beat21_delays, sizeof(beat22_notes));
	//setupMem4Track(beat23, &whereAt, beat23_notes, beat23_delays, sizeof(beat23_notes));
	
}
}