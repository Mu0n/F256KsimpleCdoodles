#define F256LIB_IMPLEMENTATION


#define TIMER_FRAMES 0
#define TIMER_SECONDS 1

 //TIMER_TEXT for the 1-frame long text refresh timer for data display
 //TIMER_NOTE is for a midi note timer
#define TIMER_TEXT_COOKIE 0



#define TIMER_TEXT_DELAY 1

#define textColorGreen  0x04
#define textColorOrange 0x09
#define textColorRed    0x91
#define textColorBlue   0x07
#define textColorGray   0x05

#include "../src/muUtils.h" //contains helper functions I often use
#include "../src/muMidi.h"  //contains basic MIDI functions I often use
#include "../src/musid.h"   //contains SID chip functions
#include "../src/mupsg.h"   //contains PSG chip functions
#include "../src/beats.h"   //contains hard coded beats

#define WITHOUT_TILE
#define WITHOUT_SPRITE
#include "f256lib.h"

EMBED(palpiano, "../assets/pian.pal", 0x20000);
EMBED(pia1, "../assets/piaa", 0x28000);
EMBED(pia2, "../assets/piab", 0x30000);
EMBED(pia3, "../assets/piac", 0x38000);


uint8_t selectBeat = 0; //selected beat preset from 0 to 3

struct timer_t spaceNotetimer, refTimer, snareTimer; //spaceNotetimer: used when you hit space, produces a 1s delay before NoteOff comes in
//refTimer: is 1 frame long, used to display updated text when you hit keys on a midi controller
//snareTimer is a pre-programmed beat at 30 frames

uint16_t note = 0x36, oldNote, oldCursorNote; /*note is the current midi hex note code to send. oldNote keeps the previous one so it can be Note_off'ed away after the timer expires, or a new note is called*/
uint16_t prgInst[10] = {0,0, 0,0,0,0,0,0,0 ,0}; /* program change value, the MIDI instrument number, for chan 0, 1 and 9=percs */
uint8_t sidInstChoice = 0; //from 0 to 5 for all 6 voices of the 2 SIDs

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
bool wantVS1053 = false; //false = SAM2695, true = VS1053b
uint8_t chipChoice = 0; //0=MIDI, 1=SID, 2=PSG, 3=OPL3

uint8_t sidChoiceToVoice[6] = {0x00, 0x07, 0x0E, 0x00, 0x07, 0x0E};
uint8_t sidInstPerVoice[6] = {0x10, 0x20, 0x40, 0x80, 0x10, 0x10};
uint8_t sidPolyCount=0; //must go down to 0 to gate off the sid. increases by +1 whenever a note is pressed. allows monophony without multitouch interference
uint8_t polyPSGcount=0;
/*
uint8_t sidLow[] = {  //from NTSC TABLE
0x0c, 0x1c, 0x2d, 0x3f, 0x52, 0x66, 0x7b, 0x92, 0xaa, 0xc3, 0xde, 0xfa, // 0 
0x18, 0x38, 0x5a, 0x7e, 0xa4, 0xcc, 0xf7, 0x24, 0x54, 0x86, 0xbc, 0xf5, // 1 
0x31, 0x71, 0xb4, 0xfc, 0x48, 0x98, 0xed, 0x48, 0xa7, 0x0c, 0x78, 0xe9, // 2 
0x62, 0xe2, 0x69, 0xf8, 0x90, 0x30, 0xdb, 0x8f, 0x4e, 0x19, 0xf0, 0xd3, // 3 
0xc4, 0xc3, 0xd1, 0xf0, 0x1f, 0x61, 0xb6, 0x1e, 0x9d, 0x32, 0xdf, 0xa6, // 4 
0x88, 0x86, 0xa3, 0xe0, 0x3f, 0xc2, 0x6b, 0x3d, 0x3a, 0x64, 0xbe, 0x4c, // 5 
0x0f, 0x0c, 0x46, 0xbf, 0x7d, 0x84, 0xd6, 0x7a, 0x73, 0xc8, 0x7d, 0x97, // 6 
0x1e, 0x18, 0x8b, 0x7f, 0xfb, 0x07, 0xac, 0xf4, 0xe7, 0x8f, 0xf9, 0x2f // 7
};
*/
/*
uint8_t sidLow[] = { //from PAL TABLE
    0x16, 0x27, 0x39, 0x4b, 0x5f, 0x74, 0x8a, 0xa1, 0xba, 0xd4, 0xf0, 0x0e, // 0
    0x2d, 0x4e, 0x71, 0x96, 0xbe, 0xe7, 0x14, 0x42, 0x74, 0xa9, 0xe0, 0x1b, // 1
    0x5a, 0x9c, 0xe2, 0x2d, 0x7b, 0xcf, 0x27, 0x85, 0xe8, 0x51, 0xc1, 0x37, // 2
    0xb4, 0x38, 0xc4, 0x59, 0xf7, 0x9d, 0x4e, 0x0a, 0xd0, 0xa2, 0x81, 0x6d, // 3
    0x67, 0x70, 0x89, 0xb2, 0xed, 0x3b, 0x9c, 0x13, 0xa0, 0x45, 0x02, 0xda, // 4
    0xce, 0xe0, 0x11, 0x64, 0xda, 0x76, 0x39, 0x26, 0x40, 0x89, 0x04, 0xb4, // 5
    0x9c, 0xc0, 0x23, 0xc8, 0xb4, 0xeb, 0x72, 0x4c, 0x80, 0x12, 0x08, 0x68, // 6
    0x39, 0x80, 0x45, 0x90, 0x68, 0xd6, 0xe3, 0x99, 0x00, 0x24, 0x10, 0xff  // 7
};
*/
/*
uint8_t sidHigh[] = {//from NTSC TABLE
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, // 0
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x03, 0x03, 0x03, 0x03, 0x03, // 1
    0x04, 0x04, 0x04, 0x04, 0x05, 0x05, 0x05, 0x06, 0x06, 0x07, 0x07, 0x07, // 2
    0x08, 0x08, 0x09, 0x09, 0x0a, 0x0b, 0x0b, 0x0c, 0x0d, 0x0e, 0x0e, 0x0f, // 3
    0x10, 0x11, 0x12, 0x13, 0x15, 0x16, 0x17, 0x19, 0x1a, 0x1c, 0x1d, 0x1f, // 4
    0x21, 0x23, 0x25, 0x27, 0x2a, 0x2c, 0x2f, 0x32, 0x35, 0x38, 0x3b, 0x3f, // 5
    0x43, 0x47, 0x4b, 0x4f, 0x54, 0x59, 0x5e, 0x64, 0x6a, 0x70, 0x77, 0x7e, // 6
    0x86, 0x8e, 0x96, 0x9f, 0xa8, 0xb3, 0xbd, 0xc8, 0xd4, 0xe1, 0xee, 0xfd  // 7
};
*/
/*
uint8_t sidHigh[] = { //from PAL TABLE
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, // 0
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x03, 0x03, 0x03, 0x03, 0x03, 0x04, // 1
    0x04, 0x04, 0x04, 0x05, 0x05, 0x05, 0x06, 0x06, 0x06, 0x07, 0x07, 0x08, // 2
    0x08, 0x09, 0x09, 0x0a, 0x0a, 0x0b, 0x0c, 0x0d, 0x0d, 0x0e, 0x0f, 0x10, // 3
    0x11, 0x12, 0x13, 0x14, 0x15, 0x17, 0x18, 0x1a, 0x1b, 0x1d, 0x1f, 0x20, // 4
    0x22, 0x24, 0x27, 0x29, 0x2b, 0x2e, 0x31, 0x34, 0x37, 0x3a, 0x3e, 0x41, // 5
    0x45, 0x49, 0x4e, 0x52, 0x57, 0x5c, 0x62, 0x68, 0x6e, 0x75, 0x7c, 0x83, // 6
    0x8b, 0x93, 0x9c, 0xa5, 0xaf, 0xb9, 0xc4, 0xd0, 0xdd, 0xea, 0xf8, 0xff  // 7
};
*/

uint8_t sidHigh[] = {
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02,
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x03, 0x03, 0x03, 0x03, 0x03, 0x04,
    0x04, 0x04, 0x04, 0x05, 0x05, 0x05, 0x06, 0x06, 0x06, 0x07, 0x07, 0x08,
    0x08, 0x08, 0x09, 0x0A, 0x0A, 0x0B, 0x0C, 0x0C, 0x0D, 0x0E, 0x0F, 0x10,
    0x10, 0x11, 0x13, 0x14, 0x15, 0x16, 0x18, 0x19, 0x1A, 0x1C, 0x1E, 0x20,
    0x21, 0x23, 0x26, 0x28, 0x2A, 0x2D, 0x30, 0x32, 0x35, 0x39, 0x3C, 0x40,
    0x43, 0x47, 0x4C, 0x50, 0x55, 0x5A, 0x60, 0x65, 0x6B, 0x72, 0x78, 0x80,
    0x87, 0x8F, 0x98, 0xA1, 0xAB, 0xB5, 0xC0, 0xCB, 0xD7, 0xE4, 0xF1, 0x00
};




uint8_t sidLow[] = {
    0x0F, 0x1F, 0x30, 0x43, 0x56, 0x6A, 0x80, 0x96, 0xAF, 0xC8, 0xE3, 0x00,
    0x1F, 0x3F, 0x61, 0x86, 0xAC, 0xD5, 0x00, 0x2D, 0x5E, 0x91, 0xC7, 0x01,
    0x3E, 0x7F, 0xC3, 0x0C, 0x58, 0xAA, 0x00, 0x5B, 0xBC, 0x23, 0x8F, 0x02,
    0x7C, 0xFE, 0x86, 0x18, 0xB1, 0x54, 0x00, 0xB7, 0x78, 0x46, 0x1F, 0x05,
    0xF9, 0xFC, 0x0D, 0x30, 0x63, 0xA8, 0x01, 0x6E, 0xF1, 0x8C, 0x3F, 0x0B,
    0xF3, 0xF8, 0x1A, 0x60, 0xC6, 0x51, 0x02, 0xDD, 0xE3, 0x18, 0x7E, 0x16,
    0xE7, 0xF0, 0x35, 0xC0, 0x8D, 0xA3, 0x05, 0xBB, 0xC7, 0x30, 0xFD, 0x2D,
    0xCE, 0xE1, 0x6A, 0x80, 0x1A, 0x46, 0x0B, 0x77, 0x8F, 0x61, 0xFA, 0x5B
};




/*
uint8_t sidLow[] = {  //corrected values
}
uint8_t sidHigh[] = { //corrected values
}
*/
const uint16_t plugin[28] = { /* Compressed plugin */
  0x0007, 0x0001, 0x8050, 0x0006, 0x0014, 0x0030, 0x0715, 0xb080, /*    0 */
  0x3400, 0x0007, 0x9255, 0x3d00, 0x0024, 0x0030, 0x0295, 0x6890, /*    8 */
  0x3400, 0x0030, 0x0495, 0x3d00, 0x0024, 0x2908, 0x4d40, 0x0030, /*   10 */
  0x0200, 0x000a, 0x0001, 0x0050
};

/*
uint8_t psgLow[] = {
	0x8A, 0x8C, 0x87, 0x88, 0x8E, 0x89, 0x12, 0x88, 0x8A,
	0x8A, 0x89, 0x85, 0x8E, 0x83, 0x8C, 0x8C, 0x85, 0x85, 0x8C, 0x8B, 0x8F,
	0x8A, 0x8A, 0x80, 0x8A, 0x8A, 0x85, 0x85, 0x88, 0x8D, 0x85, 0x8F, 0x8B,
	0x89, 0x89, 0x8B, 0x8F, 0x84, 0x8A, 0x82, 0x8C, 0x87, 0x82, 0x8F, 0x8E,
	0x8D, 0x8D, 0x8E, 0x8F, 0x82, 0x85, 0x89, 0x8E, 0x83, 0x89, 0x80, 0x87,
	0x8E, 0x86, 0x8F, 0x88, 0x81, 0x8B, 0x85, 0x8F, 0x8A, 0x85, 0x80, 0x8B,
	0x87, 0x83, 0x8F, 0x8C, 0x88, 0x85, 0x82, 0x8F, 0x8D, 0x8A, 0x88, 0x86,
	0x84, 0x82, 0x80, 0x8E, 0x8C, 0x8B
};

uint8_t psgHigh[] = {
	0x1E, 0x03, 0x2A, 0x12, 0x3B, 0x26, 0x3F, 0x2D, 0x1C, 
	0x0C, 0x3D, 0x2F, 0x21, 0x15, 0x1F, 0x16, 0x0E, 0x06, 0x3E, 0x37, 0x30,
	0x2A, 0x24, 0x1F, 0x19, 0x14, 0x35, 0x32, 0x2F, 0x2C, 0x2A, 0x27, 0x25,
	0x23, 0x21, 0x1F, 0x1D, 0x1C, 0x1A, 0x19, 0x17, 0x16, 0x15, 0x13, 0x12,
	0x11, 0x10, 0x0F, 0x0E, 0x0E, 0x0D, 0x0C, 0x0B, 0x0B, 0x0A, 0x0A, 0x09,
	0x08, 0x08, 0x07, 0x07, 0x07, 0x06, 0x06, 0x05, 0x05, 0x05, 0x05, 0x04,
	0x04, 0x04, 0x03, 0x03, 0x03, 0x03, 0x03, 0x02, 0x02, 0x02, 0x02, 0x02,
	0x02, 0x02, 0x02, 0x01, 0x01, 0x01
};
*/

uint8_t psgLow[] = {

0x86, 0x8d,0x87,0x84,0x84,0x87,0x8d,0x84,0x8e,0x8b,0x89,0x89,
0x8b, 0x8e,0x83,0x8a,0x82,0x8b,0x86,0x82,0x8f,0x8d,0x8c,0x8c,
0x8d, 0x8f,0x81,0x85,0x89,0x8d,0x83,0x89,0x8f,0x86,0x8e,0x86,
0x8e, 0x87,0x80,0x8a,0x84,0x8e,0x89,0x84,0x8f,0x8b,0x87,0x83,
0x8f, 0x8b,0x88,0x85,0x82,0x8f,0x8c,0x8a,0x87,0x85,0x83,0x81,
0x8f, 0x8d,0x8c,0x8a};


uint8_t psgHigh[] = {

0x3f, 0x3b,0x38,0x35,0x32,0x2f,0x2c,0x2a,0x27,0x25,0x23,0x21,
0x1f, 0x1d,0x1c,0x1a,0x19,0x17,0x16,0x15,0x13,0x12,0x11,0x10,
0xf, 0xe,0xe,0xd,0xc,0xb,0xb,0xa,0x9,0x9,0x8,0x8,
0x7, 0x7,0x7,0x6,0x6,0x5,0x5,0x5,0x4,0x4,0x4,0x4,
0x3, 0x3,0x3,0x3,0x3,0x2,0x2,0x2,0x2,0x2,0x2,0x2,
0x1, 0x1,0x1,0x1};

bool noteColors[88]={1,0,1, 1,0,1,0,1, 1,0,1,0,1,0,1, 1,0,1,0,1, 1,0,1,0,1,0,1, 1,0,1,0,1, 1,0,1,0,1,0,1, 1,0,1,0,1, 1,0,1,0,1,0,1, 1,0,1,0,1, 1,0,1,0,1,0,1, 1,0,1,0,1, 1,0,1,0,1,0,1, 1,0,1,0,1, 1,0,1,0,1,0,1, 1};

const char *sid_instruments[] = {
	"Triangle",
	"SawTooth",
	"Pulse",
	"Noise",
	"Misc",
	"Weird"
};
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

//this is a small code to enable the VS1053b's midi mode; present on the Jr2 and K2. It is necessary for the first revs of these 2 machines
void initVS1053MIDI(void) {
    uint8_t n;
	uint16_t addr, val, i=0;

  while (i<sizeof(plugin)/sizeof(plugin[0])) {
    addr = plugin[i++];
    n = plugin[i++];
    if (n & 0x8000U) { /* RLE run, replicate n samples */
      n &= 0x7FFF;
      val = plugin[i++];
      while (n--) {
        //WriteVS10xxRegister(addr, val);
		POKE(0xD701,addr);
		POKE(0xD701,val);
      }
    } else {           /* Copy run, copy n samples */
      while (n--) {
        val = plugin[i++];
        //WriteVS10xxRegister(addr, val);
		POKE(0xD701,addr);
		POKE(0xD701,val);
      }
    }
  }
}


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

void showMIDIChoiceText()
{
	textGotoXY(29,57);
	if(wantVS1053==false) 
		{
		if(chipChoice==0)textSetColor(textColorOrange,0x00); 
		else textSetColor(textColorGreen,0x00);
		textPrint("[MIDI sam2695]  ");
		textGotoXY(29,58);textSetColor(textColorGreen,0x00);textPrint(" MIDI VS1053b   ");
		}
	else 
		{
		textSetColor(textColorGreen,0x00); 
		textPrint(" MIDI sam2695   ");
		textGotoXY(29,58);
		if(chipChoice==0)textSetColor(textColorOrange,0x00);
		else textSetColor(textColorGreen,0x00);
		textPrint("[MIDI VS1053b]  ");
		}
}
//This swaps the text of the chip choice
void showChipChoiceText()
{
	switch(chipChoice)
	{
		case 0: //midi
			showMIDIChoiceText();
			textGotoXY(45,57);textSetColor(textColorGreen,0x00);textPrint("SID   PSG   OPL3 ");
			break;
		case 1: //SID
			showMIDIChoiceText();
			textGotoXY(44,57);textSetColor(textColorOrange,0x00);textPrint("[SID] ");
			textGotoXY(50,57);textSetColor(textColorGreen,0x00);textPrint(" PSG   OPL3  ");
			break;
		case 2: //PSG
			showMIDIChoiceText();
			textGotoXY(44,57);textSetColor(textColorGreen,0x00);textPrint(" SID  ");
			textGotoXY(50,57);textSetColor(textColorOrange,0x00);textPrint("[PSG] ");
			textGotoXY(56,57);textSetColor(textColorGreen,0x00);textPrint(" OPL3  ");
			break;
		case 3: //OPL3
			showMIDIChoiceText();
			textGotoXY(44,57);textSetColor(textColorGreen,0x00);textPrint(" SID   PSG  ");
			textGotoXY(56,57);textSetColor(textColorOrange,0x00);textPrint("[OPL3]");
			break;
	}
}


//This is the part of the text instructions and interface for selecting the sound output of the keyboard
void chipSelectTextMenu()
{
	textSetColor(textColorGreen,0x00);
	textGotoXY(5,57);textPrint("C to Cycle chip choice: ");
	textSetColor(textColorGreen,0x00); textPrint("SID   PSG   OPL3 ");
	textGotoXY(5,58);textPrint("M to toggle MIDI:");
	showChipChoiceText();
}

//This swaps the text of the midi chip choice
void showMidiChoiceText()
{
	uint8_t firstColor = textColorOrange, secondColor= textColorGreen;
	
	if(chipChoice > 0) firstColor = textColorGreen; //won't highlight any midi in orange
	else if(wantVS1053)
	{
		firstColor = textColorGreen; //reverse colors, vs highlighted in orange, sam in green
		secondColor = textColorOrange;
	}
	textSetColor(firstColor,0x00);textGotoXY(29,57);
	if(wantVS1053) textPrint(" MIDI sam2695  ");
	else textPrint("[MIDI sam2695] ");
	textSetColor(secondColor,0x00);textGotoXY(29,58);
	if(wantVS1053) textPrint("[MIDI VS1053b] ");
	else textPrint(" MIDI VS1053b  ");
	
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
	
	//Chip status, midi chip choice status
	chipSelectTextMenu();
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
	
void prepSIDinstruments()
{
	//Triangle SID 1 VOICE 1
	POKE(SID1+SID_VOICE1+SID_FM_VC, 0x0F);   // MAX VOLUME	
	POKE(SID1+SID_VOICE1+SID_LO_PWDC, 0x44); // SET PULSE WAVE DUTY LOW BYTE
	POKE(SID1+SID_VOICE1+SID_HI_PWDC, 0x00); // SET PULSE WAVE DUTY HIGH BYTE
	POKE(SID1+SID_VOICE1+SID_ATK_DEC, 0x27); // SET ATTACK;DECAY
	POKE(SID1+SID_VOICE1+SID_SUS_REL, 0x5B); // SET SUSTAIN;RELEASE
	POKE(SID1+SID_VOICE1+SID_CTRL, 0x10); 	 // SET CTRL as triangle
	
	//Sawtooth SID 1 VOICE 2
	POKE(SID1+SID_VOICE2+SID_FM_VC, 0x0F);   // MAX VOLUME	
	POKE(SID1+SID_VOICE2+SID_LO_PWDC, 0x88); // SET PULSE WAVE DUTY LOW BYTE
	POKE(SID1+SID_VOICE2+SID_HI_PWDC, 0x00); // SET PULSE WAVE DUTY HIGH BYTE
	POKE(SID1+SID_VOICE2+SID_ATK_DEC, 0x61); // SET ATTACK;DECAY
	POKE(SID1+SID_VOICE2+SID_SUS_REL, 0xC8); // SET SUSTAIN;RELEASE
	POKE(SID1+SID_VOICE2+SID_CTRL, 0x20); 	 // SET CTRL as sawtooth
	
	//Pulse SID 1 VOICE 3
	POKE(SID1+SID_VOICE3+SID_FM_VC, 0x0F);   // MAX VOLUME	
	POKE(SID1+SID_VOICE3+SID_LO_PWDC, 0x88); // SET PULSE WAVE DUTY LOW BYTE
	POKE(SID1+SID_VOICE3+SID_HI_PWDC, 0x00); // SET PULSE WAVE DUTY HIGH BYTE
	POKE(SID1+SID_VOICE3+SID_ATK_DEC, 0x61); // SET ATTACK;DECAY
	POKE(SID1+SID_VOICE3+SID_SUS_REL, 0xC8); // SET SUSTAIN;RELEASE
	POKE(SID1+SID_VOICE3+SID_CTRL, 0x40); 	 // SET CTRL as pulse
	
	//Noise SID 2 VOICE 1
	POKE(SID2+SID_VOICE1+SID_FM_VC, 0x0F);   // MAX VOLUME	
	POKE(SID2+SID_VOICE1+SID_LO_PWDC, 0x44); // SET PULSE WAVE DUTY LOW BYTE
	POKE(SID2+SID_VOICE1+SID_HI_PWDC, 0x00); // SET PULSE WAVE DUTY HIGH BYTE
	POKE(SID2+SID_VOICE1+SID_ATK_DEC, 0x17); // SET ATTACK;DECAY
	POKE(SID2+SID_VOICE1+SID_SUS_REL, 0xC8); // SET SUSTAIN;RELEASE
	POKE(SID2+SID_VOICE1+SID_CTRL, 0x80); 	 // SET CTRL as noise
	
	//Sawtooth SID 2 VOICE 2
	POKE(SID2+SID_VOICE2+SID_FM_VC, 0x0F);   // MAX VOLUME	
	POKE(SID2+SID_VOICE2+SID_LO_PWDC, 0x88); // SET PULSE WAVE DUTY LOW BYTE
	POKE(SID2+SID_VOICE2+SID_HI_PWDC, 0x00); // SET PULSE WAVE DUTY HIGH BYTE
	POKE(SID2+SID_VOICE2+SID_ATK_DEC, 0x61); // SET ATTACK;DECAY
	POKE(SID2+SID_VOICE2+SID_SUS_REL, 0xC8); // SET SUSTAIN;RELEASE
	POKE(SID2+SID_VOICE2+SID_CTRL, 0x20); 	 // SET CTRL as sawtooth
	
	//Pulse SID 2 VOICE 3
	POKE(SID2+SID_VOICE3+SID_FM_VC, 0x0F);   // MAX VOLUME	
	POKE(SID2+SID_VOICE3+SID_LO_PWDC, 0x88); // SET PULSE WAVE DUTY LOW BYTE
	POKE(SID2+SID_VOICE3+SID_HI_PWDC, 0x00); // SET PULSE WAVE DUTY HIGH BYTE
	POKE(SID2+SID_VOICE3+SID_ATK_DEC, 0x61); // SET ATTACK;DECAY
	POKE(SID2+SID_VOICE3+SID_SUS_REL, 0xC8); // SET SUSTAIN;RELEASE
	POKE(SID2+SID_VOICE3+SID_CTRL, 0x40); 	 // SET CTRL as pulse
	
	POKE(SID1+SID_LO_FCF,0x2A);
	POKE(SID1+SID_HI_FCF,0x00);
	POKE(SID1+SID_FRR, 0x00);
	POKE(SID1+SID_FM_VC, 0x0F);
		
	POKE(SID2+SID_LO_FCF,0x2A);
	POKE(SID2+SID_HI_FCF,0x00);
	POKE(SID2+SID_FRR, 0x00);
	POKE(SID2+SID_FM_VC, 0x0F);
	
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
	bitmapSetVisible(1,false);
	bitmapSetVisible(2,false);
	textTitle();
	refreshInstrumentText();

	//populate the beats
	setupBeats();
	
	
	//prep the tempoLUT
	prepTempoLUT();
	
	//Prep all the kernel timers	
	refTimer.units = TIMER_FRAMES;
	refTimer.absolute = getTimerAbsolute(TIMER_FRAMES) + TIMER_TEXT_DELAY;
	refTimer.cookie = TIMER_TEXT_COOKIE;
	setTimer(&refTimer);

	resetInstruments(wantVS1053);
	POKE(wantVS1053?MIDI_FIFO_ALT:MIDI_FIFO,0xC2);
	POKE(wantVS1053?MIDI_FIFO_ALT:MIDI_FIFO,0x73);//woodblock
	prgInst[0]=0;prgInst[1]=0;prgInst[9]=0;
	textSetColor(textColorOrange,0x00);
	
	clearSIDRegisters();
	prepSIDinstruments();
	
	printf("Initiating VS1053b ");
	initVS1053MIDI();
	
}

//This function highlights or de-highlights a choice in Instrument Picking mode
void highLightInstChoice(bool isNew)
{
	uint8_t x, y;
	isNew?textSetColor(textColorOrange,0):textSetColor(textColorGray,0);
	
	if(chipChoice==0) //MIDI
	{
		x= 2 + 25 * (prgInst[chSelect]%3);
		y= 1 + (prgInst[chSelect]/3);
		textGotoXY(x,y);printf("%003d %s",prgInst[chSelect],midi_instruments[prgInst[chSelect]]);
	}
	if(chipChoice==1) //SID
	{
		x= 2;
		y= 2 + sidInstChoice;
		textGotoXY(x,y);printf("%003d %s",sidInstChoice,sid_instruments[sidInstChoice]);
	}
}

//These four next functions are for moving around your selection in Instrument Picking mode
void modalMoveUp(bool shift)
{
	highLightInstChoice(false);
	if(chipChoice==0)prgInst[chSelect] -= (3 + shift*27);
	if(chipChoice==1)sidInstChoice--;
	highLightInstChoice(true);
}
void modalMoveDown(bool shift)
{
	highLightInstChoice(false);
	if(chipChoice==0)prgInst[chSelect] += (3 + shift*27);
	if(chipChoice==1)sidInstChoice++;
	highLightInstChoice(true);
}
void modalMoveLeft()
{
	highLightInstChoice(false);
	if(chipChoice==0)prgInst[chSelect] -= 1;
	if(chipChoice==1)sidInstChoice--;
	highLightInstChoice(true); 
}
void modalMoveRight()
{
	highLightInstChoice(false);
	if(chipChoice==0)prgInst[chSelect] += 1;
	if(chipChoice==1)sidInstChoice++;
	highLightInstChoice(true);
}

//In this Instrument Picking mode called by hitting [F1], display all General MIDI instruments in 3 columns
//and highlight the currently activated one for the selected channel
void instListShow()
{
	uint8_t i, y=1;
	if(chipChoice==0){
	midiShutAChannel(0, wantVS1053);
	midiShutAChannel(1, wantVS1053);
	midiShutAChannel(9, wantVS1053);
	}
	if(chipChoice==1) shutAllSIDVoices();
	realTextClear();
	
	textSetColor(textColorOrange,0x00);
	if(chipChoice==0) //MIDI
	{
		textGotoXY(0,0);textPrint("Select your instrument for channel");printf(" %d",chSelect);textPrint(". [Arrows] [Enter] [Space] [Back]");
		textSetColor(textColorGray,0x00);
		for(i=0; i<(sizeof(midi_instruments)/sizeof(midi_instruments[0]));i++)
		{
			textGotoXY(2,y);printf("%003d %s ",i,midi_instruments[i]);
			i++;
			textGotoXY(27,y);printf("%003d %s ",i,midi_instruments[i]);
			i++;if(i==sizeof(midi_instruments)/sizeof(midi_instruments[0])) break;
			textGotoXY(52,y);printf("%003d %s ",i,midi_instruments[i]);
			y++;
		}
	}
	if(chipChoice==1) //SID
	{
		textGotoXY(0,0);textPrint("Select your instrument for SID. [Arrows] [Enter] [Space] [Back]");
		textSetColor(textColorGray,0x00);
		for(i=0; i<(sizeof(sid_instruments)/sizeof(sid_instruments[0]));i++)
		{
			textGotoXY(2,1+y+i);printf("%003d %s ",i,sid_instruments[i]);

		}
	}
	highLightInstChoice(true);
}

//dispatchNote deals with all possibilities
void dispatchNote(bool isOn, uint8_t channel, uint8_t note, uint8_t speed, bool wantAlt)
{
	uint16_t sidTarget, sidVoiceBase;
	
	if(chipChoice==0) //MIDI
	{
		if(isOn) {
			midiNoteOn(channel, note, speed, wantAlt);
			if(isTwinLinked) midiNoteOn(1, note,speed, wantAlt);
		}
		else 
		{
			midiNoteOff(channel, note, speed, wantAlt);
			if(isTwinLinked) midiNoteOff(1, note,speed, wantAlt);
		}
		return;
	}
	if(chipChoice==1) //SID
	{
		if(sidInstChoice>=0 && sidInstChoice<3) sidTarget = SID1; //means SID 1 
		else sidTarget = SID2; //otherwise SID2
		
		sidVoiceBase = sidTarget + sidChoiceToVoice[sidInstChoice];
		if(isOn)
		{
			sidPolyCount+=1;
			POKE(sidVoiceBase+SID_LO_B, sidLow[note-11]); // SET FREQUENCY FOR NOTE 1 
			POKE(sidVoiceBase+SID_HI_B, sidHigh[note-11]); // SET FREQUENCY FOR NOTE 1 
			sidNoteOnOrOff(sidVoiceBase+SID_CTRL, sidInstPerVoice[sidInstChoice], isOn);
		}
		else 
		{
			sidPolyCount-=1;
			if(sidPolyCount==0) 
			{
				POKE(sidVoiceBase+SID_LO_B, sidLow[note-11]); // SET FREQUENCY FOR NOTE 1 
				POKE(sidVoiceBase+SID_HI_B, sidHigh[note-11]); // SET FREQUENCY FOR NOTE 1 
				sidNoteOnOrOff(sidVoiceBase+SID_CTRL, sidInstPerVoice[sidInstChoice], isOn);
			}
		}
		return;
	}
	if(chipChoice==2) //PSG
	{
		//psgNoteOff();
		if(isOn) {
			psgNoteOn(psgLow[note-45],psgHigh[note-45]);
			polyPSGcount++;
			}
		else 
		{
			polyPSGcount--;
			if(polyPSGcount==0)	psgNoteOff();
		}
		return;
	}
}


int main(int argc, char *argv[]) {
	uint16_t toDo, sidPitchBend;
	uint8_t i,j;
	uint8_t recByte, detectedNote, detectedColor;
	bool nextIsNote = false; //detect a 0x9? or 0x8? command, the next is a note byte, used for coloring the keyboard note-rects
	bool nextIsSpeed = false; //detects if we're at the end of a note on or off trio of bytes
	bool nextIsBend = false, nextIsLastBend=false; //detects if we're about to pitch bend
	bool isHit = false; // true is hit, false is released
	bool altHit = false, shiftHit = false; //keyboard modifiers
	bool instSelectMode = false; //is currently in the mode where you see the whole instrument list
	POKE(1,0);
	uint8_t storedNote; //stored values when twinlinked
    bool needsToWaitExpiration = false;  //when tempo is changed, must wait to expire all pending timers before sounding again.

	setup();
	textGotoXY(0,2);
	note=39;
	oldCursorNote=39;
	
	graphicsDefineColor(0, note+0x61,0xFF,0x00,0x00); 
	
	while(true) 
        {
		if(!(PEEK(wantVS1053?MIDI_CTRL_ALT:MIDI_CTRL) & 0x02)) //rx not empty
			{
				toDo = PEEKW(wantVS1053?MIDI_RXD_ALT:MIDI_RXD) & 0x0FFF; //discard top 4 bits of MIDI_RXD+1
				
				//erase the region where the last midi bytes received are shown
				if(instSelectMode==false){
					textGotoXY(5,4);textPrint("                                                                                ");textGotoXY(5,4);
				}
				//deal with the MIDI bytes and exhaust the FIFO buffer
				for(i=0; i<toDo; i++)
				{
					//get the next MIDI in FIFO buffer byte
					recByte=PEEK(wantVS1053?MIDI_FIFO_ALT:MIDI_FIFO);
					if(instSelectMode==false)
					{
						textSetColor(textColorOrange,0x00);printf("%02x ",recByte); //output midi byte on screen at the top
					}
					if(nextIsSpeed) //this block activates when a note is getting finished on the 3rd byte ie 0x90 0x39 0x80 (noteOn middleC midSpeed)
					{
						nextIsSpeed = false;
						if(isHit) {
							dispatchNote(true, chSelect,storedNote,recByte<0x70?0x70:recByte, wantVS1053);
						}
						
						else {
							dispatchNote(false, chSelect,storedNote,recByte<0x70?0x70:recByte, wantVS1053);
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
					if(nextIsLastBend) //bend in a SID choice context
					{
						nextIsLastBend=false;
					}
					if(nextIsBend) //bend in a SID choice context
					{
						nextIsBend=false;
						nextIsLastBend=true;
						POKE(sidChoiceToVoice[sidInstChoice]+SID_LO_PWDC, recByte);
					}
					if((recByte & 0xF0 )== 0x90) { //we know it's a 'NoteOn', get ready to analyze the note byte, which is next
						nextIsNote = true;
						isHit=true;
						if(chipChoice==0) POKE(wantVS1053?MIDI_FIFO_ALT:MIDI_FIFO, ((recByte & 0xF0 ) | chSelect)); //send it to the chosen channel, only the first byte, and reroute
						

						
					}
					else if((recByte & 0xF0  )== 0x80) { //we know it's a 'NoteOff', get ready to analyze the note byte, which is next
						nextIsNote = true;
						isHit=false;
						if(chipChoice==0) POKE(wantVS1053?MIDI_FIFO_ALT:MIDI_FIFO, ((recByte & 0xF0 ) | chSelect)); //send it to the chosen channel, only the first byte, and reroute
						}
					else if((recByte & 0xF0) == 0xE0 && chipChoice == 1) { //we know it's a pitch bend incoming
						nextIsBend = true;
					}
					else if(chipChoice==0) POKE(wantVS1053?MIDI_FIFO_ALT:MIDI_FIFO, recByte); //all other bytes are sent normally
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
				case  TIMER_BEAT_0:
					if(theBeats[selectBeat].isActive)
					{
						midiNoteOff(theBeats[selectBeat].channel[0], //channel
								    theBeats[selectBeat].notes[0][theBeats[selectBeat].index[0]], //note
									0x7F, //speed
									wantVS1053 //midi chip selection
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
													   0x7F,                         //speed
														wantVS1053); //midi chip selection);								    
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
									0x7F, //speed
									wantVS1053 //midi chip selection
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
									0x7F, //speed
									wantVS1053); //midi chip selection
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
													   0x7F,								//speed
													   wantVS1053);							//midi chip selection	    
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
									0x7F, //speed
									wantVS1053); //midi chip selection
						theBeats[selectBeat].activeCount+=1;
						theBeats[selectBeat].timers[j].absolute = getTimerAbsolute(TIMER_FRAMES) + mainTempoLUT[theBeats[selectBeat].delays[j][theBeats[selectBeat].index[j]]];
						setTimer(&(theBeats[selectBeat].timers[j]));
						}
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
						resetInstruments(wantVS1053);	
						POKE(wantVS1053?MIDI_FIFO_ALT:MIDI_FIFO,0xC2);
						POKE(wantVS1053?MIDI_FIFO_ALT:MIDI_FIFO,0x73);//woodblock
						prgInst[0]=0;prgInst[1]=0;prgInst[9]=0;						
						realTextClear();
						refreshInstrumentText();
						textTitle();
						shutAllSIDVoices();
						instSelectMode=false; //returns to default mode
						break;
					case 129: //F1
						instSelectMode = !instSelectMode; //toggle this mode
						if(instSelectMode == true) instListShow();
						else if(instSelectMode==false) {
							realTextClear();
							textTitle();
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
							midiShutAChannel(0, wantVS1053);
							midiShutAChannel(1, wantVS1053);
							midiShutAChannel(9, wantVS1053);
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
											   0x7F,								//speed
											   wantVS1053);							//midi chip selection    
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
								if(chipChoice==0)
								{
									if(prgInst[chSelect] < 127 - shiftHit*9) prgInst[chSelect] = prgInst[chSelect] + 1 + shiftHit *9; //go up 10 instrument ticks if shift is on, otherwise just 1
									if(altHit) prgInst[chSelect] = 127; //go to highest instrument, 127
									prgChange(prgInst[chSelect],chSelect, wantVS1053);
								}
								if(chipChoice==1)
								{
									if(sidInstChoice<5) sidInstChoice++;
								}
								refreshInstrumentText();
							}
							else if(instSelectMode==true)
							{
								if(chipChoice==0)
								{
									if(prgInst[chSelect] > (shiftHit?29:2)) modalMoveUp(shiftHit);
									prgChange(prgInst[chSelect],chSelect, wantVS1053);
								}
								if(chipChoice==1 && sidInstChoice>0) modalMoveUp(shiftHit);
							}	
						break;
					case 0xb7: //down arrow
						if(instSelectMode==false)
								{
								if(chipChoice==0)
									{
									if(prgInst[chSelect] > 0 + shiftHit*9) prgInst[chSelect] = prgInst[chSelect] - 1 - shiftHit *9; //go down 10 instrument ticks if shift is on, otherwise just 1
									if(altHit) prgInst[chSelect] = 0; //go to lowest instrument, 0
									prgChange(prgInst[chSelect],chSelect, wantVS1053);
									}
								if(chipChoice==1)
									{
										if(sidInstChoice>0) sidInstChoice--;
									}
									refreshInstrumentText();
								}
							else if(instSelectMode==true)
							{
								if(chipChoice==0)
									{
										if(prgInst[chSelect] < (shiftHit?98:125)) modalMoveDown(shiftHit);
										prgChange(prgInst[chSelect],chSelect, wantVS1053);
									}
								if(chipChoice==1 && sidInstChoice<5) modalMoveDown(shiftHit);
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
							prgChange(prgInst[chSelect],chSelect, wantVS1053);
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
							prgChange(prgInst[chSelect],chSelect, wantVS1053);
							}
						break;
					case 32: //space
							//Send a Note
							dispatchNote(true, 0x90 | chSelect ,note+0x15,0x7F, wantVS1053);		
							
							//keep track of that note so we can Note_Off it when needed
							oldNote = note+0x15; //make it possible to do the proper NoteOff when the timer expires


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
							
							midiShutAChannel(0, wantVS1053);
							midiShutAChannel(1, wantVS1053);
							midiShutAChannel(9, wantVS1053);
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
							
							midiShutAChannel(0, wantVS1053);
							midiShutAChannel(1, wantVS1053);
							midiShutAChannel(9, wantVS1053);
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
							midiShutAChannel(0, wantVS1053);
							midiShutAChannel(1, wantVS1053);
							midiShutAChannel(9, wantVS1053);
						}
						break;
					case 109: // M - toggle the MIDI chip between default SAM2695 to VS1053b
						wantVS1053 = !wantVS1053;
						showMidiChoiceText();
						break;
					case 99: // C - chip select mode: 0=MIDI, 1=SID, (todo) 2= PSG, (todo) 3=OPL3
						chipChoice+=1;
						if(chipChoice==1) prepSIDinstruments(); //just arrived in sid, prep sid
						if(chipChoice==2) clearSIDRegisters(); //just left sid, so clear sid
						if(chipChoice==3) psgNoteOff(); //just left PSG, so clear PSG
						if(chipChoice>3)chipChoice=0; //loop back to midi at the start of the cyle
						showChipChoiceText();
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
				case 32: //space
					if(chipChoice==1) sidPolyCount=1;
					dispatchNote(false, chSelect ,note+0x15,0x3F, wantVS1053);						
					break;
			}
		}		
        }
return 0;}
}