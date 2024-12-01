#include "D:\F256\llvm-mos\code\midiTest\.builddir\trampoline.h"

#define F256LIB_IMPLEMENTATION

#define MIDI_CTRL 	   0xDDA0
#define MIDI_FIFO 	   0xDDA1
#define MIDI_RXD 	   0xDDA2
#define MIDI_RXD_COUNT 0xDDA3
#define MIDI_TXD       0xDDA4
#define MIDI_TXD_COUNT 0xDDA5

#define TIMER_FRAMES 0
#define TIMER_SECONDS 1

 //TIMER_TEXT for the 1-frame long text refresh timer for data display
 //TIMER_NOTE is for a midi note timer
#define TIMER_TEXT_COOKIE 0

#define TIMER_BEAT_0 5
#define TIMER_BEAT_1A 6
#define TIMER_BEAT_1B 7
#define TIMER_BEAT_2A 8
#define TIMER_BEAT_2B 9
#define TIMER_BEAT_3A 10
#define TIMER_BEAT_3B 11

#define TIMER_SPACENOTE_COOKIE 50 //will be 50-99

#define TIMER_TEXT_DELAY 1
#define TIMER_SPACENOTE_DELAY 60

#define textColorGreen  0x04
#define textColorOrange 0x09
#define textColorRed    0x91
#define textColorBlue   0x07
#define textColorGray   0x05


#include "../src/muUtils.h" //contains helper functions I often use
#include "../src/muMidi.h"  //contains basic MIDI functions I often use
#define WITHOUT_TILE
#define WITHOUT_SPRITE
#include "f256lib.h"

EMBED(palpiano, "../assets/pian.pal", 0x20000);
EMBED(pia1, "../assets/piaa", 0x28000);
EMBED(pia2, "../assets/piab", 0x30000);
EMBED(pia3, "../assets/piac", 0x38000);

struct aBeat
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
};

struct aBeat theBeats[4];
uint8_t selectBeat = 0; //selected beat preset from 0 to 3

struct timer_t spaceNotetimer, refTimer, snareTimer; //spaceNotetimer: used when you hit space, produces a 1s delay before NoteOff comes in
//refTimer: is 1 frame long, used to display updated text when you hit keys on a midi controller
//snareTimer is a pre-programmed beat at 30 frames

uint16_t note = 0x36, oldNote, oldCursorNote; /*note is the current midi hex note code to send. oldNote keeps the previous one so it can be Note_off'ed away after the timer expires, or a new note is called*/
uint16_t prgInst[10] = {0,0, 0,0,0,0,0,0,0 ,0}; /* program change value, the MIDI instrument number, for chan 0, 1 and 9=percs */

uint8_t chSelect = 0; //restricted to channel 0, 1 or 9 (for percs)
//lowest note on 88-key piano is a A more than 3 octaves below middle CLUT
//midi note number of that lowest note is dec=21

//reference tempoLUT, close to 112.5 bpm where a quarter note = 32 frames = 0,533s
uint8_t tempoLUTRef[18] = {4, 8, 16, 32, 64, 128, //         32nd, 16th, 8th, 4th, half, whole
						   8,12, 24, 48, 96, 192, //dotted   32nd, 16th, 8th, 4th, half, whole
					      	11, 21, 32, 5, 11, 16}; //triplets of 4th: lengths of 1, 2, 3,  triplets of 8ths of length 1, 2, 3
uint8_t mainTempo = 120;
uint8_t mainTempoLUT[18];
bool isTwinLinked = false; //when true, sends out midi notes to both channels when using a midi controller or space bar
	

bool noteColors[88]={1,0,1, 1,0,1,0,1, 1,0,1,0,1,0,1, 1,0,1,0,1, 1,0,1,0,1,0,1, 1,0,1,0,1, 1,0,1,0,1,0,1, 1,0,1,0,1, 1,0,1,0,1,0,1, 1,0,1,0,1, 1,0,1,0,1,0,1, 1,0,1,0,1, 1,0,1,0,1,0,1, 1,0,1,0,1, 1,0,1,0,1,0,1, 1};

const char *midi_instruments[] = {
    "Ac. Grand Piano",
    "Bright Ac. Piano",
    "Electric Grand Piano",
    "Honky-tonk Piano",
    "Electric Piano 1",
    "Electric Piano 2",
    "Harpsichord",
    "Clavinet",
    "Celesta",
    "Glockenspiel",
    "Music Box",
    "Vibraphone",
    "Marimba",
    "Xylophone",
    "Tubular Bells",
    "Santur",
    "Drawbar Organ",
    "Percussive Organ",
    "Rock Organ",
    "Church Organ",
    "Reed Organ",
    "Accordion",
    "Harmonica",
    "Tango Accordion",
    "Ac. Guitar (nylon)",
    "Ac. Guitar (steel)",
    "Elec. Guitar (jazz)",
    "Elec. Guitar (clean)",
    "Elec. Guitar (muted)",
    "Overdriven Guitar",
    "Distortion Guitar",
    "Guitar harmonics",
    "Acoustic Bass",
    "Elec. Bass (finger)",
    "Elec. Bass (pick)",
    "Fretless Bass",
    "Slap Bass 1",
    "Slap Bass 2",
    "Synth Bass 1",
    "Synth Bass 2",
    "Violin",
    "Viola",
    "Cello",
    "Contrabass",
    "Tremolo Strings",
    "Pizzicato Strings",
    "Orchestral Harp",
    "Timpani",
    "String Ensemble 1",
    "String Ensemble 2",
    "SynthStrings 1",
    "SynthStrings 2",
    "Choir Aahs",
    "Voice Oohs",
    "Synth Voice",
    "Orchestra Hit",
    "Trumpet",
    "Trombone",
    "Tuba",
    "Muted Trumpet",
    "French Horn",
    "Brass Section",
    "SynthBrass 1",
    "SynthBrass 2",
    "Soprano Sax",
    "Alto Sax",
    "Tenor Sax",
    "Baritone Sax",
    "Oboe",
    "English Horn",
    "Bassoon",
    "Clarinet",
    "Piccolo",
    "Flute",
    "Recorder",
    "Pan Flute",
    "Blown Bottle",
    "Shakuhachi",
    "Whistle",
    "Ocarina",
    "Lead 1 (square)",
    "Lead 2 (sawtooth)",
    "Lead 3 (calliope)",
    "Lead 4 (chiff)",
    "Lead 5 (charang)",
    "Lead 6 (voice)",
    "Lead 7 (fifths)",
    "Lead 8 (bass + lead)",
    "Pad 1 (new age)",
    "Pad 2 (warm)",
    "Pad 3 (polysynth)",
    "Pad 4 (choir)",
    "Pad 5 (bowed)",
    "Pad 6 (metallic)",
    "Pad 7 (halo)",
    "Pad 8 (sweep)",
    "FX 1 (rain)",
    "FX 2 (soundtrack)",
    "FX 3 (crystal)",
    "FX 4 (atmosphere)",
    "FX 5 (brightness)",
    "FX 6 (goblins)",
    "FX 7 (echoes)",
    "FX 8 (sci-fi)",
    "Sitar",
    "Banjo",
    "Shamisen",
    "Koto",
    "Kalimba",
    "Bag pipe",
    "Fiddle",
    "Shanai",
    "Tinkle Bell",
    "Agogo",
    "Steel Drums",
    "Woodblock",
    "Taiko Drum",
    "Melodic Tom",
    "Synth Drum",
    "Reverse Cymbal",
    "Guitar Fret Noise",
    "Breath Noise",
    "Seashore",
    "Bird Tweet",
    "Telephone Ring",
    "Helicopter",
    "Applause",
    "Gunshot"
};


//This is part of the text instructions and interface during regular play mode
void refreshInstrumentText()
{
	textGotoXY(5,30); textPrint("                                   ");
	textGotoXY(5,31); textPrint("                                   ");
	
	textGotoXY(5,30); (chSelect==0)?textSetColor(textColorOrange,0x00):textSetColor(textColorGreen,0x00);
	printf("%03d %s",prgInst[0],midi_instruments[prgInst[0]]);
	textGotoXY(5,31); (chSelect==1)?textSetColor(textColorOrange,0x00):textSetColor(textColorGreen,0x00);
	printf("%03d %s",prgInst[1],midi_instruments[prgInst[1]]);
	textGotoXY(5,32); (chSelect==9)?textSetColor(textColorOrange,0x00):textSetColor(textColorGreen,0x00);
	textPrint("Percussion");
	
	if(isTwinLinked){
		textGotoXY(5,31);
		textSetColor(textColorOrange,0x00);
		printf("%03d %s",prgInst[1],midi_instruments[prgInst[1]]);
		}
	
	textSetColor(textColorOrange,0x00);
	switch(chSelect)
	{
		case 0:
			textGotoXY(0,30);printf("%c",0xFA);
			textGotoXY(0,31);textPrint(" ");
			textGotoXY(0,32);textPrint(" ");
			break;
		case 1:
			textGotoXY(0,30);textPrint(" ");
			textGotoXY(0,31);printf("%c",0xFA);
			textGotoXY(0,32);textPrint(" ");
			break;
		case 9:
			textGotoXY(0,30);textPrint(" ");
			textGotoXY(0,31);textPrint(" ");
			textGotoXY(0,32);printf("%c",0xFA);
			break;
	}
	if(isTwinLinked)
	{
			textGotoXY(0,30);printf("%c",0xFA);
			textGotoXY(0,31);printf("%c",0xFA);
			textGotoXY(0,32);textPrint(" ");
	}
}

//This is part of the text instructions and interface during regular play mode
void channelTextMenu()
{
	textSetColor(textColorGreen,0x00);
	
	textGotoXY(0,26);textPrint("[F1] to pick an instrument from a list");
	textGotoXY(0,27);textPrint("[F3] to change your output channel");
	textGotoXY(1,29);textPrint("CH  Instrument");
	textGotoXY(2,30);textPrint("0: ");
	textGotoXY(2,31);textPrint("1: ");
	textGotoXY(2,32);textPrint("9: ");
	textGotoXY(0,33);textPrint("[X] to twin link channels 0 & 1");
	refreshInstrumentText();
}


void updateTempoText()
{
	textSetColor(textColorGreen,0x00);
	textGotoXY(45,31);textPrint("Tempo BPM: ");textPrintInt(mainTempo);textPrint("  ");
	textGotoXY(45,32);textPrint("[: -1 ]: +1 Sh-[: -10  Sh-]: +10");
}


void refreshBeatTextChoice()
{
	switch(selectBeat)
		{
			case 0: //beat1
				textSetColor(textColorOrange,0x00);
				textGotoXY(45,29); textPrint("[WaveS] ");
				textSetColor(textColorGreen,0x00);textPrint(" Da Da   Jazzy   Funky ");
				break;
			case 1: //beat2
				textSetColor(textColorGreen,0x00);
				textGotoXY(45,29); textPrint(" WaveS  ");
				textSetColor(textColorOrange,0x00);
				textPrint("[Da Da] ");
				textSetColor(textColorGreen,0x00);
				textPrint(" Jazzy   Funky ");
				break;
			case 2: //beat3
				textSetColor(textColorGreen,0x00);
				textGotoXY(45,29); textPrint(" WaveS   Da Da  ");
				textSetColor(textColorOrange,0x00);
				textPrint("[Jazzy] ");
				textSetColor(textColorGreen,0x00);
				textPrint(" Funky ");
				break;
			case 3: //Funky
				textSetColor(textColorGreen,0x00);
				textGotoXY(45,29); textPrint(" WaveS   Da Da   Jazzy  ");
				textSetColor(textColorOrange,0x00);
				textPrint("[Funky]");
				break;
		}
}

//This is part of the text instructions and interface during regular play mode
void beatsTextMenu()
{
	textSetColor(textColorGreen,0x00);
	updateTempoText();
	
	textGotoXY(45,26);textPrint("[F5] to cycle preset beats");
	textGotoXY(45,27);textPrint("[F7] play/stop toggle the beat");
	refreshBeatTextChoice();
}



//This restores the full text instructions and interface during regular play mode
void textTitle()
{
	uint16_t c;
	//Text Title area
	textSetColor(textColorRed,0x00);
	textGotoXY(17,0);for(c=8;c>0;c--) printf("%c",0x15+c);
	textGotoXY(26,0);printf("Mu0n's MIDI test v1.0");
	textGotoXY(48,0);for(c=1;c<9;c++) printf("%c",0x15+c);
    textDefineForegroundColor(0,0xff,0xff,0xff);
	textSetColor(textColorBlue,0x00);
	textGotoXY(0,2); textPrint("Plug in a midi controller in the MIDI IN port and play!");
	channelTextMenu();
	textSetColor(textColorBlue,0x00);

	beatsTextMenu();

	//Instrument selection instructions
	textGotoXY(0,35);printf("[%c] / [%c] to change the instrument",0xF8,0xFB);
	textGotoXY(0,36);printf("[Shift-%c] / [Shift-%c] to move by 10 - [Alt-%c] / [Alt-%c] go to the ends ",0xF8,0xFB,0xF8,0xFB);

	//Note play through spacebar instructions
	textGotoXY(0,40);textPrint("[Space] to play a short note under the red line cursor");
	textGotoXY(0,41);printf("[%c] / [%c] to move the cursor",0xF9,0xFA);
	textGotoXY(0,42);printf("[Shift-%c] / [Shift-%c] to move an octave - [Alt-%c] / [Alt-%c] go to the ends ",0xF9,0xFA,0xF9,0xFA);
}

//setupBeats: will hard code some presets for the beats
void setupBeats()
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

void prepTempoLUT()
{
	uint8_t t;
	for(t=0;t<18;t++) {
		mainTempoLUT[t] = (uint8_t)(1.0f*112.5f*tempoLUTRef[t]/(1.0f*mainTempo));
	}
	mainTempoLUT[12] = mainTempoLUT[14]-mainTempoLUT[13];
	//corrects rounding error and makes these triplets compatible with a quarter note
	
	mainTempoLUT[15] = mainTempoLUT[17]-mainTempoLUT[16];
	//same with triplet eights
}
	
//setup is called just once during initial launching of the program
void setup()
{
	uint16_t c;
	realTextClear();
	
	POKE(MMU_IO_CTRL, 0x00);
	POKE(VKY_MSTR_CTRL_0, 0x4F); //graphics and tile enabled
	POKE(VKY_MSTR_CTRL_1, 0x00); //320x240 at 60 Hz; double height text
	POKE(VKY_LAYER_CTRL_0, 0x00); //bitmap 0 in layer 0, bitmap 1 in layer 1
	
	POKE(MMU_IO_CTRL,1);  //MMU I/O to page 1
	//prep to copy over the palette to the CLUT
	for(c=0;c<1023;c++)
	{
		POKE(VKY_GR_CLUT_0+c, FAR_PEEK(0x20000+c)); //palette for piano
	}
	
	POKE(MMU_IO_CTRL,0); //MMU I/O to page 0
	
	//piano bitmap at layer 0
	bitmapSetActive(0);
	bitmapSetAddress(0,0x28000);
	bitmapSetVisible(0,true);
	bitmapSetCLUT(0);
	
	textTitle();
	refreshInstrumentText();

	//populate the beats
	setupBeats();
	
	
	//prep the tempoLUT
	prepTempoLUT();
	
	//Prep all the kernel timers

	spaceNotetimer.units = TIMER_FRAMES;
    spaceNotetimer.cookie = TIMER_SPACENOTE_COOKIE;
	
	refTimer.units = TIMER_FRAMES;
	refTimer.absolute = getTimerAbsolute(TIMER_FRAMES) + TIMER_TEXT_DELAY;
	refTimer.cookie = TIMER_TEXT_COOKIE;
	setTimer(&refTimer);

	resetInstruments();
	POKE(MIDI_FIFO,0xC2);
	POKE(MIDI_FIFO,0x73);//woodblock
	prgInst[0]=0;prgInst[1]=0;prgInst[9]=0;
	textSetColor(textColorOrange,0x00);
}

//This function highlights or de-highlights a choice in Instrument Picking mode
void highLightInstChoice(bool isNew)
{
	uint8_t x, y;
	isNew?textSetColor(textColorOrange,0):textSetColor(textColorGray,0);
	x= 2 + 25 * (prgInst[chSelect]%3);
	y= 1 + (prgInst[chSelect]/3);
	textGotoXY(x,y);printf("%003d %s",prgInst[chSelect],midi_instruments[prgInst[chSelect]]);
}

//These four next functions are for moving around your selection in Instrument Picking mode
void modalMoveUp(bool shift)
{
	highLightInstChoice(false);
	prgInst[chSelect] -= (3 + shift*27);
	highLightInstChoice(true);
}
void modalMoveDown(bool shift)
{
	highLightInstChoice(false);
	prgInst[chSelect] += (3 + shift*27);
	highLightInstChoice(true);
}
void modalMoveLeft()
{
	highLightInstChoice(false);
	prgInst[chSelect] -= 1;
	highLightInstChoice(true);
}
void modalMoveRight()
{
	highLightInstChoice(false);
	prgInst[chSelect] += 1;
	highLightInstChoice(true);
}

//In this Instrument Picking mode called by hitting [F1], display all General MIDI instruments in 3 columns
//and highlight the currently activated one for the selected channel
void instListShow()
{
	uint8_t i, y=1;
	midiShutAChannel(0);
	midiShutAChannel(1);
	midiShutAChannel(9);
	realTextClear();
	
	textSetColor(textColorOrange,0x00);
	textGotoXY(0,0);textPrint("Select your instrument for channel");printf(" %d",chSelect);textPrint(". [Arrows] [Enter] [Space] [Back]");
	textSetColor(textColorGray,0x00);
	for(i=0; i<128; i++)
	{
		textGotoXY(2,y);printf("%003d %s ",i,midi_instruments[i]);
		i++;
		textGotoXY(27,y);printf("%003d %s ",i,midi_instruments[i]);
		i++;if(i==128)break;
		textGotoXY(52,y);printf("%003d %s ",i,midi_instruments[i]);
		y++;
	}
	highLightInstChoice(true);
}

int main(int argc, char *argv[]) {
	uint16_t toDo;
	uint8_t i,j;
	uint8_t recByte, detectedNote, detectedColor;
	bool nextIsNote = false; //detect a 0x9? or 0x8? command, the next is a note byte, used for coloring the keyboard note-rects
	bool nextIsSpeed = false; //detects if we're at the end of a note on or off trio of bytes
	bool isHit = false; // true is hit, false is released
	bool altHit = false, shiftHit = false; //keyboard modifiers
	uint8_t spaceNoteCookieOffset =0; //used to pile on cookies for spacebar activated notes
	bool instSelectMode = false; //is currently in the mode where you see the whole instrument list
	POKE(1,0);
	uint8_t storedNote, storedSpeed; //stored values when twinlinked
    bool needsToWaitExpiration = false;  //when tempo is changed, must wait to expire all pending timers before sounding again.
	setup();
	textGotoXY(0,2);
	note=39;
	oldCursorNote=39;
	
	graphicsDefineColor(0, note+0x61,0xFF,0x00,0x00);
	
	while(true)
        {
		if(!(PEEK(MIDI_CTRL) & 0x02)) //rx not empty
			{
				toDo = PEEKW(MIDI_RXD) & 0x0FFF; //discard top 4 bits of MIDI_RXD+1
	
				//erase the region where the last midi bytes received are shown
				if(instSelectMode==false){
					textGotoXY(5,4);textPrint("                                                                                ");textGotoXY(5,4);
				}
				//deal with the MIDI bytes and exhaust the FIFO buffer
				for(i=0; i<toDo; i++)
				{
					//get and print byte by byte
					recByte=PEEK(MIDI_FIFO);
					if(instSelectMode==false)
					{
						textSetColor(textColorOrange,0x00);printf("%02x ",recByte);
					}
					if(nextIsSpeed) //this block activates when a note is getting finished
					{
						nextIsSpeed = false;
						if(isHit) {
							midiNoteOn(chSelect,storedNote,recByte<0x70?0x70:recByte);
							if(isTwinLinked) midiNoteOn(1, storedNote,recByte<0x70?0x70:recByte);
						}
	
						else {
							midiNoteOff(chSelect,storedNote,recByte<0x70?0x70:recByte);
							if(isTwinLinked) midiNoteOff(1, storedNote,recByte<0x70?0x70:recByte);
							}
					}
					if(nextIsNote) //this block triggers if the previous byte was a NoteOn or NoteOff (0x90,0x80) command previously
					{
						//figure out which note on the graphic is going to be highlighted
						detectedNote = recByte-0x14;
	
						//first case is when the last command is a 0x90 'NoteOn' command
						if(isHit) graphicsDefineColor(0, detectedNote,0xFF,0x00,0xFF); //paint it as a hit note
 						//otherwise it's a 0x80 'NoteOff' command
						else {
							detectedColor = noteColors[detectedNote-1]?0xFF:0x00;
							graphicsDefineColor(0, detectedNote,detectedColor,detectedColor,detectedColor); //swap back the original color according to this look up ref table noteColors
						}
						nextIsNote = false; //return to previous state after a note is being dealt with
						storedNote = recByte;
						nextIsSpeed = true;
					}
					if((recByte & 0xF0 )== 0x90) { //we know it's a 'NoteOn', get ready to analyze the note byte, which is next
						nextIsNote = true;
						isHit=true;
						POKE(MIDI_FIFO, ((recByte & 0xF0 ) | chSelect)); //send it to the chosen channel, only the first byte, and reroute
	
					}
					else if((recByte & 0xF0  )== 0x80) { //we know it's a 'NoteOff', get ready to analyze the note byte, which is next
						nextIsNote = true;
						isHit=false;
						POKE(MIDI_FIFO, ((recByte & 0xF0 ) | chSelect)); //send it to the chosen channel, only the first byte, and reroute
						}
					else POKE(MIDI_FIFO, recByte); //all other bytes are sent normally
						//analysis of the byte is done, send it back for consumption
				}
			}
	
		kernelNextEvent();
        if(kernelEventData.type == kernelEvent(timer.EXPIRED))
            {
			switch(kernelEventData.timer.cookie)
				{
				//all user interface related to text update through a 1 frame timer is managed here
				case TIMER_TEXT_COOKIE:
					refTimer.absolute = getTimerAbsolute(TIMER_FRAMES) + TIMER_TEXT_DELAY;
					setTimer(&refTimer);
					break;
				//all programmatically note playing is expired here
				case TIMER_SPACENOTE_COOKIE ... (TIMER_SPACENOTE_COOKIE+49):
							spaceNoteCookieOffset--;
							if(spaceNoteCookieOffset==0)
							{
							//Send a Note_Off midi command for the previously ongoing note
							POKE(MIDI_FIFO, 0x80 | chSelect);
							POKE(MIDI_FIFO, oldNote);
							POKE(MIDI_FIFO, 0x4F);
							}
					break;
				case  TIMER_BEAT_0:
					if(theBeats[selectBeat].isActive)
					{
						midiNoteOff(theBeats[selectBeat].channel[0], //channel
								    theBeats[selectBeat].notes[0][theBeats[selectBeat].index[0]], //note
									0x7F //speed
						);
						theBeats[selectBeat].activeCount-=1;
						//check if we need to expire the beat and act to die things down
						if(needsToWaitExpiration)
							{
								if(theBeats[selectBeat].activeCount > 0)
								{
									break;
								}
								else
								{
									needsToWaitExpiration = false;
									theBeats[selectBeat].isActive = false;
									if(theBeats[selectBeat].pendingRelaunch) //oops, need to relaunch it at the end of it dying down
									{
										theBeats[selectBeat].pendingRelaunch = false; //the relaunch will happen, so stop further relaunches for now
										theBeats[selectBeat].isActive = true;
										//mainTempo = theBeats[selectBeat].suggTempo;
										prepTempoLUT();
										updateTempoText();
										for(j=0;j<theBeats[selectBeat].howMany;j++)
										{
											theBeats[selectBeat].index[j] = 0;
											midiNoteOn(theBeats[selectBeat].channel[j],  //channel
													   theBeats[selectBeat].notes[j][0],    //note
													   0x7F);								    //speed
											theBeats[selectBeat].activeCount+=1;
											theBeats[selectBeat].timers[j].absolute = getTimerAbsolute(TIMER_FRAMES) + 		mainTempoLUT[theBeats[selectBeat].delays[j][0]];
											setTimer(&(theBeats[selectBeat].timers[j]));
										}
									}
									break;
								}
							}
	
						//otherwise proceed as normal and get the next note of that beat's track
						theBeats[selectBeat].index[0]+=1;
	
						if(theBeats[selectBeat].index[0] == theBeats[selectBeat].noteCount[0]) theBeats[selectBeat].index[0] = 0;
	
						midiNoteOn(theBeats[selectBeat].channel[0], //channel
								    theBeats[selectBeat].notes[0][theBeats[selectBeat].index[0]], //note
									0x7F //speed
						);
						theBeats[selectBeat].activeCount+=1;
						theBeats[selectBeat].timers[0].absolute = getTimerAbsolute(TIMER_FRAMES) + mainTempoLUT[
																							theBeats[selectBeat].delays[0][theBeats[selectBeat].index[0]]
																						];
	
						setTimer(&(theBeats[selectBeat].timers[0]));
					}
					break;
				case TIMER_BEAT_1A ... TIMER_BEAT_3B:
					if(theBeats[selectBeat].isActive)
					{
						switch(kernelEventData.timer.cookie)
						{
						case TIMER_BEAT_1A:
						case TIMER_BEAT_1B:
							j = kernelEventData.timer.cookie -TIMER_BEAT_1A; //find the right track of the beat to deal with
							break;
						case TIMER_BEAT_2A:
						case TIMER_BEAT_2B:
							j = kernelEventData.timer.cookie -TIMER_BEAT_2A;
							break;
						case TIMER_BEAT_3A:
						case TIMER_BEAT_3B:
							j = kernelEventData.timer.cookie -TIMER_BEAT_3A;
							break;
						}
	
						midiNoteOff(theBeats[selectBeat].channel[j], //channel
								    theBeats[selectBeat].notes[j][theBeats[selectBeat].index[j]], //note
									0x7F //speed
									);
						theBeats[selectBeat].activeCount-=1;
						//check if we need to expire the beat and act to die things down
						if(needsToWaitExpiration)
							{
								if(theBeats[selectBeat].activeCount > 0)
								{
									break;
								}
								else
								{
									needsToWaitExpiration = false;
									theBeats[selectBeat].isActive = false;
									if(theBeats[selectBeat].pendingRelaunch) //oops, need to relaunch it at the end of it dying down
									{
										theBeats[selectBeat].pendingRelaunch = false; //the relaunch will happen, so stop further relaunches for now
										theBeats[selectBeat].isActive = true;
										//mainTempo = theBeats[selectBeat].suggTempo;
										prepTempoLUT();
										updateTempoText();
										for(j=0;j<theBeats[selectBeat].howMany;j++)
										{
											theBeats[selectBeat].index[j] = 0;
											midiNoteOn(theBeats[selectBeat].channel[j],  //channel
													   theBeats[selectBeat].notes[j][0],    //note
													   0x7F);								    //speed
											theBeats[selectBeat].activeCount+=1;
											theBeats[selectBeat].timers[j].absolute = getTimerAbsolute(TIMER_FRAMES) + 		mainTempoLUT[theBeats[selectBeat].delays[j][0]];
											setTimer(&(theBeats[selectBeat].timers[j]));
										}
									}
									break;
								}
							}
						//otherwise proceed as normal and get the next note of that beat's track
						theBeats[selectBeat].index[j]+=1;
						if(theBeats[selectBeat].index[j] == theBeats[selectBeat].noteCount[j]) theBeats[selectBeat].index[j] = 0;
						midiNoteOn(theBeats[selectBeat].channel[j], //channel
								    theBeats[selectBeat].notes[j][theBeats[selectBeat].index[j]], //note
									0x7F //speed
									);
						theBeats[selectBeat].activeCount+=1;
						theBeats[selectBeat].timers[j].absolute = getTimerAbsolute(TIMER_FRAMES) + mainTempoLUT[theBeats[selectBeat].delays[j][theBeats[selectBeat].index[j]]];
						setTimer(&(theBeats[selectBeat].timers[j]));
						}
					break;
					/*
				case TIMER_SNARE_COOKIE:
					if(autoSnare)
						{
						POKE(MIDI_FIFO, 0x89);
						POKE(MIDI_FIFO, snareNotes[sIndex]);
						POKE(MIDI_FIFO, 0x4F);
						sIndex++;
						if(sIndex==4)sIndex=0;
						POKE(MIDI_FIFO, 0x99);
						POKE(MIDI_FIFO, snareNotes[sIndex]);
						POKE(MIDI_FIFO, 0x7F);
						snareTimer.absolute = getTimerAbsolute(TIMER_FRAMES) + snareDelays[sIndex];
						setTimer(&snareTimer);
						}
						*/
					break;
				}
            }
		else if(kernelEventData.type == kernelEvent(key.PRESSED))
			{
				switch(kernelEventData.key.raw)
				{
					case 148: //enter
							if(instSelectMode){
								realTextClear();
								textTitle();
								instSelectMode=false;
							}
							break;
					case 146: // top left backspace, meant as reset
						resetInstruments();
						POKE(MIDI_FIFO,0xC2);
						POKE(MIDI_FIFO,0x73);//woodblock
						prgInst[0]=0;prgInst[1]=0;prgInst[9]=0;
						realTextClear();
						refreshInstrumentText();
						textTitle();
						instSelectMode=false; //returns to default mode
						break;
					case 129: //F1
						instSelectMode = !instSelectMode; //toggle this mode
						if(instSelectMode == true) instListShow();
						else if(instSelectMode==false) {
							realTextClear();
							textTitle();
							//prgChange(prgInst[chSelect],chSelect);
							}
						break;
					case 131: //F3
						if(instSelectMode==false) {
							isTwinLinked=false;
							chSelect++;
							if(chSelect == 2)
								{
								chSelect = 9;
								isTwinLinked=false;
							}
							if(chSelect == 10) chSelect = 0;
							channelTextMenu();
							refreshInstrumentText();
							midiShutAChannel(0);
							midiShutAChannel(1);
							midiShutAChannel(9);
						}
						break;
					case 133: //F5
						if(instSelectMode==false) {
	
							if(theBeats[selectBeat].isActive) needsToWaitExpiration = true; //orders a dying down of the beat
							if(needsToWaitExpiration) break; //we're not done finishing the beat, do nothing new
	
							if(theBeats[selectBeat].isActive == false) //cycles the beat
								{
								selectBeat+=1;
								if(selectBeat==4)selectBeat=0;
								refreshBeatTextChoice();
								}
						}
						break;
					case 135: //F7
						if(instSelectMode==false) {
	
							if(theBeats[selectBeat].isActive) needsToWaitExpiration = true; //orders a dying down of the beat
							if(needsToWaitExpiration) break; //we're not done finishing the beat, do nothing new
	
							if(theBeats[selectBeat].isActive == false) //starts the beat
							{
								theBeats[selectBeat].isActive = true;
								mainTempo = theBeats[selectBeat].suggTempo;
								prepTempoLUT();
								updateTempoText();
								for(j=0;j<theBeats[selectBeat].howMany;j++)
								{
									theBeats[selectBeat].index[j] = 0;
									midiNoteOn(theBeats[selectBeat].channel[j],  //channel
											   theBeats[selectBeat].notes[j][0],    //note
											   0x7F);								    //speed
									theBeats[selectBeat].activeCount+=1;
									theBeats[selectBeat].timers[j].absolute = getTimerAbsolute(TIMER_FRAMES) + 		mainTempoLUT[theBeats[selectBeat].delays[j][0]];
									setTimer(&(theBeats[selectBeat].timers[j]));
								}
							}
						}
						break;
					case 0xb6: //up arrow
						if(instSelectMode==false)
							{
								if(prgInst[chSelect] < 127 - shiftHit*9) prgInst[chSelect] = prgInst[chSelect] + 1 + shiftHit *9; //go up 10 instrument ticks if shift is on, otherwise just 1
								if(altHit) prgInst[chSelect] = 127; //go to highest instrument, 127
								prgChange(prgInst[chSelect],chSelect);
								refreshInstrumentText();
							}
							else if(instSelectMode==true)
							{
								if(prgInst[chSelect] > (shiftHit?29:2)) modalMoveUp(shiftHit);
								prgChange(prgInst[chSelect],chSelect);
							}
						break;
					case 0xb7: //down arrow
						if(instSelectMode==false)
								{
								if(prgInst[chSelect] > 0 + shiftHit*9) prgInst[chSelect] = prgInst[chSelect] - 1 - shiftHit *9; //go down 10 instrument ticks if shift is on, otherwise just 1
								if(altHit) prgInst[chSelect] = 0; //go to lowest instrument, 0
								prgChange(prgInst[chSelect],chSelect);
								refreshInstrumentText();
								}
							else if(instSelectMode==true)
							{
								if(prgInst[chSelect] < (shiftHit?98:125)) modalMoveDown(shiftHit);
								prgChange(prgInst[chSelect],chSelect);
							}
						break;
					case 0xb8: //left arrow
						if(instSelectMode==false)
								{
								if(note > (0 + shiftHit*11)) note = note - 1 - shiftHit * 11;
								if(altHit) note = 0; //go to the leftmost note
								graphicsDefineColor(0, oldCursorNote+0x61,0x00,0x00,0x00); //remove old cursor position
								graphicsDefineColor(0, note+0x61,0xFF,0x00,0x00); //set new cursor position
								oldCursorNote = note;
								}
						else if(instSelectMode==true)
							{
							if(prgInst[chSelect] > 0) modalMoveLeft();
							prgChange(prgInst[chSelect],chSelect);
							}
						break;
					case 0xb9: //right arrow
						if(instSelectMode==false)
							{
							if(note < 87 - shiftHit*11) note = note + 1 + shiftHit * 11;
							if(altHit) note = 87; //go to the rightmost note
	
							graphicsDefineColor(0, oldCursorNote+0x61,0x00,0x00,0x00);//remove old cursor position
							graphicsDefineColor(0, note+0x61,0xFF,0x00,0x00); //set new cursor position
							oldCursorNote = note;
							}
						else if(instSelectMode==true)
							{
							if(prgInst[chSelect] < 127) modalMoveRight();
							prgChange(prgInst[chSelect],chSelect);
							}
						break;
					case 32: //space
							//this command will send a programmed note of the current instrument on channel 0, of note value from the global
							//a timer will expire and auto-dial in its NoteOff command

							//Send a Note_Off midi command for the previously ongoing note
	
							midiShutAChannel(0);
							midiShutAChannel(1);
							midiShutAChannel(9);
							//Send a Note_On midi command on channel 0
							midiNoteOn(0x90 | chSelect, note+0x15, 0x7F);
							if(isTwinLinked) midiNoteOn(0x91, note+0x15, 0x4F);

							//keep track of that note so we can Note_Off it when needed
							oldNote = note+0x15; //make it possible to do the proper NoteOff when the timer expires
	
							//Prepare the next note timer
							spaceNotetimer.cookie = TIMER_SPACENOTE_COOKIE + spaceNoteCookieOffset; //possible pile up of number used to represent the cookie
							spaceNotetimer.absolute = getTimerAbsolute(TIMER_FRAMES) + TIMER_SPACENOTE_DELAY;
							setTimer(&spaceNotetimer);
	
							spaceNoteCookieOffset++;
						break;
					case 5: //alt modifier
						altHit = true;
						break;
					case 1: //shift modifier
						shiftHit = true;
						break;
					case 91: // '['
						if(theBeats[selectBeat].isActive) {
							mainTempo -= (1 + shiftHit*9);
							prepTempoLUT();
							updateTempoText();
	
							needsToWaitExpiration = true; //orders a dying down of the beat
							theBeats[selectBeat].pendingRelaunch=true;
						}
						if(needsToWaitExpiration) break; //we're not done finishing the beat, do nothing new
	
						if(theBeats[selectBeat].isActive == false) //changes the tempo
						{
							needsToWaitExpiration=true; //order an expiration of the beat
							break; //not done expiring a beat, don't start a new one
						}
						//proceed to change tempo freely if one isn't playing at all
						else if(instSelectMode==false && mainTempo > 30 + shiftHit*9) {
							mainTempo -= (1 + shiftHit*9);
							prepTempoLUT();
							updateTempoText();
	
							midiShutAChannel(0);
							midiShutAChannel(1);
							midiShutAChannel(9);
							for(j=0;j<theBeats[selectBeat].howMany;j++) theBeats[selectBeat].index[j]=0;
						}
						break;
					case 93: // ']'
						if(theBeats[selectBeat].isActive) {
							mainTempo += (1 + shiftHit*9);
							prepTempoLUT();
							updateTempoText();
							needsToWaitExpiration = true; //orders a dying down of the beat
							theBeats[selectBeat].pendingRelaunch=true;
						}
						if(needsToWaitExpiration) break; //we're not done finishing the beat, do nothing new
	
						if(theBeats[selectBeat].isActive)
							{
								needsToWaitExpiration=true; //order an expiration of the beat
								break; //not done expiring a beat, don't start a new one
							}
						else if(instSelectMode==false && mainTempo < 255 - shiftHit*9) {
							mainTempo += (1 + shiftHit*9);
							prepTempoLUT();
							updateTempoText();
	
							midiShutAChannel(0);
							midiShutAChannel(1);
							midiShutAChannel(9);
							for(j=0;j<theBeats[selectBeat].howMany;j++) theBeats[selectBeat].index[j]=0;
						}
						break;
					case 120: // X - twin link mode
						if(instSelectMode==false) {
							isTwinLinked = !isTwinLinked;
							if(isTwinLinked) {
								chSelect=0;
								textSetColor(textColorOrange,0x00);
								textGotoXY(0,30);printf("%c",0xFA);
								textGotoXY(0,31);printf("%c",0xFA);
								textGotoXY(0,32);textPrint(" ");
							} else
							{
								chSelect=0;
								textSetColor(textColorOrange,0x00);
								textGotoXY(0,30);printf("%c",0xFA);
								textGotoXY(0,31);textPrint(" ");
								textGotoXY(0,32);textPrint(" ");
							}
	
							channelTextMenu();
							refreshInstrumentText();
							midiShutAChannel(0);
							midiShutAChannel(1);
							midiShutAChannel(9);
						}
						break;
				}
				//the following line can be used to get keyboard codes
				//printf("\n %d",kernelEventData.key.raw);
			}
		else if(kernelEventData.type == kernelEvent(key.RELEASED))
		{
			switch(kernelEventData.key.raw)
			{
				case 5:  //alt modifier
					altHit = false;
					break;
				case 1: //shift modifier
					shiftHit = false;
					break;
			}
		}
        }
return 0;}
