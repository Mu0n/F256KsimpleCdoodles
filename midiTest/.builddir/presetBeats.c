#include "D:\F256\llvm-mos\code\midiTest\.builddir\trampoline.h"

#include "f256lib.h"
#include "../src/presetBeats.h"
#include "../src/timerDefs.h"

//assumes you've malloc'ed it beforehand!
void setupBeats(struct aB *theBeats)
{
	//Beat 0, simple kick drum + snare beat
	theBeats[0].isActive = false;
	theBeats[0].activeCount=0;
	theBeats[0].suggTempo = 90;
	theBeats[0].howMany = 1;
	theBeats[0].channel = malloc(sizeof(uint8_t) * 1);
	theBeats[0].channel[0] = 0x09; //percussion
	theBeats[0].index = malloc(sizeof(uint8_t) * 1);
	theBeats[0].index[0] = 0;
	theBeats[0].timers = malloc(sizeof(struct timer_t) * 1);
	theBeats[0].timers[0].units = TIMER_FRAMES;
	theBeats[0].timers[0].cookie = TIMER_BEAT_0;
	theBeats[0].notes = (uint8_t **)malloc(1 * sizeof(uint8_t *)); //1 track
	theBeats[0].delays = (uint8_t **)malloc(1 * sizeof(uint8_t *));//1 track
	theBeats[0].notes[0] = 	malloc(sizeof(uint8_t) * 2); //just 2 notes!
	theBeats[0].delays[0] = malloc(sizeof(uint8_t) * 2); //just 2 notes!
	theBeats[0].notes[0][0] = 0x24; //kick drum
	theBeats[0].notes[0][1] = 0x26; //snare drum
	theBeats[0].delays[0][0] = 3; //3rd element of tempo LUT, quarter notes
	theBeats[0].delays[0][1] = 3;
	theBeats[0].noteCount = malloc(sizeof(uint8_t) * 1);
	theBeats[0].noteCount[0] = 2;
	
	//Beat 1, famous casio beat used in Da Da Da
	theBeats[1].isActive = false;
	theBeats[1].activeCount=0;
	theBeats[1].suggTempo = 128;
	theBeats[1].howMany = 2;
	theBeats[1].channel = malloc(sizeof(uint8_t) * 2); // percs and woodblock, 2 channels
	theBeats[1].channel[0] = 0x09; //percussion
	theBeats[1].channel[1] = 0x02; //woodblock
	theBeats[1].index = malloc(sizeof(uint8_t) * 2);
	theBeats[1].index[0] = theBeats[1].index[1] = 0;
	theBeats[1].timers = malloc(sizeof(struct timer_t) * 2);
	theBeats[1].timers[0].units = TIMER_FRAMES;
	theBeats[1].timers[1].units = TIMER_FRAMES;
	theBeats[1].timers[0].cookie = TIMER_BEAT_1A;
	theBeats[1].timers[1].cookie = TIMER_BEAT_1B;
	theBeats[1].notes = (uint8_t **)malloc(2 * sizeof(uint8_t *)); // 2 tracks
	theBeats[1].delays = (uint8_t **)malloc(2 * sizeof(uint8_t *)); // 2 tracks
	theBeats[1].notes[0] = malloc(sizeof(uint8_t) * 5);
	theBeats[1].delays[0] = malloc(sizeof(uint8_t) * 5);
	
	theBeats[1].notes[1] = malloc(sizeof(uint8_t) * 8);
	theBeats[1].delays[1] = malloc(sizeof(uint8_t) * 8);
	
	theBeats[1].noteCount = malloc(sizeof(uint8_t) * 2);
	theBeats[1].noteCount[0]=5;
	theBeats[1].noteCount[1]=8;
	
	theBeats[1].notes[0][0] =  0x24; //kick drum
	theBeats[1].notes[0][1] =  0x28; //snare drum
	theBeats[1].notes[0][2] =  0x28;
	theBeats[1].notes[0][3] =  0x24;
	theBeats[1].notes[0][4] =  0x28;
	
	theBeats[1].delays[0][0] =  3;
	theBeats[1].delays[0][1] =  2;
	theBeats[1].delays[0][2] =  2;
	theBeats[1].delays[0][3] =  3;
	theBeats[1].delays[0][4] =  3;
	
	theBeats[1].noteCount[0] = 5;
	
	theBeats[1].notes[1][0] =  0x4B;
	theBeats[1].notes[1][1] =  0x63;
	theBeats[1].notes[1][2] =  0x00;
	theBeats[1].notes[1][3] =  0x63;
	theBeats[1].notes[1][4] =  0x4B;
	theBeats[1].notes[1][5] =  0x63;
	theBeats[1].notes[1][6] =  0x4B;
	theBeats[1].notes[1][7] =  0x63;
	
	theBeats[1].delays[1][0] =  2;
	theBeats[1].delays[1][1] =  2;
	theBeats[1].delays[1][2] =  2;
	theBeats[1].delays[1][3] =  2;
	theBeats[1].delays[1][4] =  2;
	theBeats[1].delays[1][5] =  2;
	theBeats[1].delays[1][6] =  2;
	theBeats[1].delays[1][7] =  2;
	
	theBeats[1].noteCount[1] = 8;
	
	
	//Beat 2, jazz cymbal ride
	theBeats[2].isActive = false;
	theBeats[2].activeCount=0;
	theBeats[2].suggTempo = 100;
	theBeats[2].howMany = 2;
	theBeats[2].channel = malloc(sizeof(uint8_t) * 2); // 2 tracks all percs
	theBeats[2].channel[0] = 0x09; //cymbal
	theBeats[2].channel[1] = 0x09; //kick snare
	theBeats[2].index = malloc(sizeof(uint8_t) * 2);
	theBeats[2].index[0] = 0;
	theBeats[2].index[1] = 0;
	theBeats[2].timers = malloc(sizeof(struct timer_t) * 2);
	theBeats[2].timers[0].units = TIMER_FRAMES;
	theBeats[2].timers[0].cookie = TIMER_BEAT_2A;
	theBeats[2].timers[1].units = TIMER_FRAMES;
	theBeats[2].timers[1].cookie = TIMER_BEAT_2B;
	theBeats[2].notes = (uint8_t **)malloc(2 * sizeof(uint8_t *)); //2 track
	theBeats[2].delays = (uint8_t **)malloc(2 * sizeof(uint8_t *));//2 track
	theBeats[2].noteCount = malloc(sizeof(uint8_t) * 2);//2 track
	
	theBeats[2].notes[0] = 	malloc(sizeof(uint8_t) * 6); //6 notes!
	theBeats[2].notes[1] = 	malloc(sizeof(uint8_t) * 4); //4 notes!
	theBeats[2].delays[0] = malloc(sizeof(uint8_t) * 4); //6 notes!
	theBeats[2].delays[1] = malloc(sizeof(uint8_t) * 6); //6 notes!
	
	theBeats[2].noteCount[0] = 4;
	theBeats[2].notes[0][0] = 0x24;
	theBeats[2].notes[0][1] = 0x26;
	theBeats[2].notes[0][2] = 0x24;
	theBeats[2].notes[0][3] = 0x26;
	theBeats[2].delays[0][0] = 3;
	theBeats[2].delays[0][1] = 3;
	theBeats[2].delays[0][2] = 3;
	theBeats[2].delays[0][3] = 3;
	
	theBeats[2].noteCount[1] = 6;
	theBeats[2].notes[1][0] = 0x39;
	theBeats[2].notes[1][1] = 0x39;
	theBeats[2].notes[1][2] = 0x39;
	theBeats[2].notes[1][3] = 0x39;
	theBeats[2].notes[1][4] = 0x39;
	theBeats[2].notes[1][5] = 0x39;
	theBeats[2].delays[1][0] = 3;
	theBeats[2].delays[1][1] = 13;
	theBeats[2].delays[1][2] = 12;
	theBeats[2].delays[1][3] = 3;
	theBeats[2].delays[1][4] = 13;
	theBeats[2].delays[1][5] = 12;
	
	
	//Beat 3, funk swung 16th note
	theBeats[3].isActive = false;
	theBeats[3].activeCount=0;
	theBeats[3].suggTempo = 80;
	theBeats[3].howMany = 2;
	theBeats[3].channel = malloc(sizeof(uint8_t) * 2); // 2 tracks all percs
	theBeats[3].channel[0] = 0x09; //hit hat
	theBeats[3].channel[1] = 0x09; //kick, rim
	theBeats[3].index = malloc(sizeof(uint8_t) * 2);
	theBeats[3].index[0] = 0;
	theBeats[3].index[1] = 0;
	theBeats[3].timers = malloc(sizeof(struct timer_t) * 2);
	theBeats[3].timers[0].units = TIMER_FRAMES;
	theBeats[3].timers[0].cookie = TIMER_BEAT_3A;
	theBeats[3].timers[1].units = TIMER_FRAMES;
	theBeats[3].timers[1].cookie = TIMER_BEAT_3B;
	theBeats[3].notes = (uint8_t **)malloc(2 * sizeof(uint8_t *)); //2 track
	theBeats[3].delays = (uint8_t **)malloc(2 * sizeof(uint8_t *));//2 track
	theBeats[3].noteCount = malloc(sizeof(uint8_t) * 2); //2 track

	theBeats[3].notes[0] = 	malloc(sizeof(uint8_t) * 32); //32 notes!
	theBeats[3].notes[1] = 	malloc(sizeof(uint8_t) * 12); //8 notes!
	theBeats[3].delays[0] = malloc(sizeof(uint8_t) * 32); //32 notes!
	theBeats[3].delays[1] = malloc(sizeof(uint8_t) * 12); //8 notes!
	
	theBeats[3].noteCount[0] = 32;
	theBeats[3].notes[0][0]  = 0x2C;
	theBeats[3].notes[0][1]  = 0x2C;
	theBeats[3].notes[0][2]  = 0x2C;
	theBeats[3].notes[0][3]  = 0x2C;
	theBeats[3].notes[0][4]  = 0x2C;
	theBeats[3].notes[0][5]  = 0x2C;
	theBeats[3].notes[0][6]  = 0x2C;
	theBeats[3].notes[0][7]  = 0x2C;
	theBeats[3].notes[0][8]  = 0x2C;
	theBeats[3].notes[0][9]  = 0x2C;
	theBeats[3].notes[0][10] = 0x2C;
	theBeats[3].notes[0][11] = 0x2C;
	theBeats[3].notes[0][12] = 0x2C;
	theBeats[3].notes[0][13] = 0x2C;
	theBeats[3].notes[0][14] = 0x2C;
	theBeats[3].notes[0][15] = 0x2C;
	
	theBeats[3].notes[0][16] = 0x2C;
	theBeats[3].notes[0][17] = 0x2C;
	theBeats[3].notes[0][18] = 0x2C;
	theBeats[3].notes[0][19] = 0x2C;
	theBeats[3].notes[0][20] = 0x2C;
	theBeats[3].notes[0][21] = 0x2C;
	theBeats[3].notes[0][22] = 0x2C;
	theBeats[3].notes[0][23] = 0x2C;
	theBeats[3].notes[0][24] = 0x2C;
	theBeats[3].notes[0][25] = 0x2C;
	theBeats[3].notes[0][26] = 0x2E;
	theBeats[3].notes[0][27] = 0x2E;
	theBeats[3].notes[0][28] = 0x2C;
	theBeats[3].notes[0][29] = 0x2C;
	theBeats[3].notes[0][30] = 0x2C;
	theBeats[3].notes[0][31] = 0x2C;
	
	theBeats[3].delays[0][0]  = 16;
	theBeats[3].delays[0][1]  = 15;
	theBeats[3].delays[0][2]  = 16;
	theBeats[3].delays[0][3]  = 15;
	theBeats[3].delays[0][4]  = 16;
	theBeats[3].delays[0][5]  = 15;
	theBeats[3].delays[0][6]  = 16;
	theBeats[3].delays[0][7]  = 15;
	theBeats[3].delays[0][8]  = 16;
	theBeats[3].delays[0][9]  = 15;
	theBeats[3].delays[0][10] = 16;
	theBeats[3].delays[0][11] = 15;
	theBeats[3].delays[0][12] = 16;
	theBeats[3].delays[0][13] = 15;
	theBeats[3].delays[0][14] = 16;
	theBeats[3].delays[0][15] = 15;
	
	theBeats[3].delays[0][16] = 16;
	theBeats[3].delays[0][17] = 15;
	theBeats[3].delays[0][18] = 16;
	theBeats[3].delays[0][19] = 15;
	theBeats[3].delays[0][20] = 16;
	theBeats[3].delays[0][21] = 15;
	theBeats[3].delays[0][22] = 16;
	theBeats[3].delays[0][23] = 15;
	theBeats[3].delays[0][24] = 16;
	theBeats[3].delays[0][25] = 15;
	theBeats[3].delays[0][26] = 16;
	theBeats[3].delays[0][27] = 15;
	theBeats[3].delays[0][28] = 16;
	theBeats[3].delays[0][29] = 15;
	theBeats[3].delays[0][30] = 16;
	theBeats[3].delays[0][31] = 15;
	
	theBeats[3].noteCount[1] = 12; //alt: floor tom:0x2D or kick drum 0x24; rim: 0x25 snare 0x26
	theBeats[3].notes[1][0]  = 0x24;
	theBeats[3].notes[1][1]  = 0x00;
	theBeats[3].notes[1][2]  = 0x24;
	theBeats[3].notes[1][3]  = 0x26;
	theBeats[3].notes[1][4]  = 0x00;
	theBeats[3].notes[1][5]  = 0x24;
	theBeats[3].notes[1][6]  = 0x00;
	theBeats[3].notes[1][7]  = 0x24;
	theBeats[3].notes[1][8]  = 0x24;
	theBeats[3].notes[1][9]  = 0x26;
	theBeats[3].notes[1][10] = 0x00;
	theBeats[3].notes[1][11] = 0x24;
	theBeats[3].delays[1][0]  = 17;
	theBeats[3].delays[1][1]  = 16;
	theBeats[3].delays[1][2]  = 15;
	theBeats[3].delays[1][3]  = 17;
	theBeats[3].delays[1][4]  = 16;
	theBeats[3].delays[1][5]  = 15;
	theBeats[3].delays[1][6]  = 16;
	theBeats[3].delays[1][7]  = 15;
	theBeats[3].delays[1][8]  = 17;
	theBeats[3].delays[1][9]  = 17;
	theBeats[3].delays[1][10] = 16;
	theBeats[3].delays[1][11] = 15;
}
