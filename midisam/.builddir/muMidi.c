#include "D:\F256\llvm-mos\code\midisam\.builddir\trampoline.h"

#include "f256lib.h"
#include "../src/muMidi.h"

const char *midi_instruments[128] = {
    "Ac. Grand Piano",    "Bright Ac. Piano",    "Electric Grand Piano", "Honky-tonk Piano",
    "Electric Piano 1",    "Electric Piano 2",    "Harpsichord",    "Clavinet",
    "Celesta",    "Glockenspiel",    "Music Box",    "Vibraphone",
	"Marimba",    "Xylophone",    "Tubular Bells",    "Santur",
    "Drawbar Organ",    "Percussive Organ",    "Rock Organ",    "Church Organ",
    "Reed Organ",    "Accordion",    "Harmonica",    "Tango Accordion",
    "Ac. Guitar (nylon)",    "Ac. Guitar (steel)",    "Elec. Guitar (jazz)",    "Elec. Guitar (clean)",
    "Elec. Guitar (muted)",    "Overdriven Guitar",    "Distortion Guitar",    "Guitar harmonics",
    "Acoustic Bass",    "Elec. Bass (finger)",    "Elec. Bass (pick)",    "Fretless Bass",
    "Slap Bass 1",    "Slap Bass 2",    "Synth Bass 1",    "Synth Bass 2",
    "Violin",    "Viola",    "Cello",    "Contrabass",
    "Tremolo Strings",    "Pizzicato Strings",    "Orchestral Harp",    "Timpani",
    "String Ensemble 1",    "String Ensemble 2",    "SynthStrings 1",    "SynthStrings 2",
    "Choir Aahs",    "Voice Oohs",    "Synth Voice",    "Orchestra Hit",
    "Trumpet",    "Trombone",    "Tuba",    "Muted Trumpet",
    "French Horn",    "Brass Section",    "SynthBrass 1",    "SynthBrass 2",
    "Soprano Sax",    "Alto Sax",    "Tenor Sax",    "Baritone Sax",
    "Oboe",    "English Horn",    "Bassoon",    "Clarinet",
    "Piccolo",    "Flute",    "Recorder",    "Pan Flute",
    "Blown Bottle",    "Shakuhachi",    "Whistle",    "Ocarina",
    "Lead 1 (square)",    "Lead 2 (sawtooth)",    "Lead 3 (calliope)",    "Lead 4 (chiff)",
    "Lead 5 (charang)",    "Lead 6 (voice)",    "Lead 7 (fifths)",    "Lead 8 (bass + lead)",
    "Pad 1 (new age)",    "Pad 2 (warm)",    "Pad 3 (polysynth)",    "Pad 4 (choir)",
    "Pad 5 (bowed)",    "Pad 6 (metallic)",    "Pad 7 (halo)",    "Pad 8 (sweep)",
    "FX 1 (rain)",    "FX 2 (soundtrack)",    "FX 3 (crystal)",    "FX 4 (atmosphere)",
    "FX 5 (brightness)",    "FX 6 (goblins)",    "FX 7 (echoes)",    "FX 8 (sci-fi)",
	"Sitar",    "Banjo",    "Shamisen",    "Koto",
    "Kalimba",    "Bag pipe",    "Fiddle",    "Shanai",
    "Tinkle Bell",    "Agogo",    "Steel Drums",    "Woodblock",
    "Taiko Drum",    "Melodic Tom",    "Synth Drum",    "Reverse Cymbal",
    "Guitar Fret Noise",    "Breath Noise",    "Seashore",    "Bird Tweet",
    "Telephone Ring",    "Helicopter",    "Applause",    "Gunshot"
};

static const uint16_t plugin[28] = { /* Compressed plugin  for the VS1053b to enable real time midi mode */
  0x0007, 0x0001, 0x8050, 0x0006, 0x0014, 0x0030, 0x0715, 0xb080, /*    0 */
  0x3400, 0x0007, 0x9255, 0x3d00, 0x0024, 0x0030, 0x0295, 0x6890, /*    8 */
  0x3400, 0x0030, 0x0495, 0x3d00, 0x0024, 0x2908, 0x4d40, 0x0030, /*   10 */
  0x0200, 0x000a, 0x0001, 0x0050
};


// wantAlt = true if you want to use the VS1053b, false if you want to use the sam2695

//shut one channel in particular from 0 to 15
void midiShutAChannel(uint8_t chan, bool wantAlt)
{
	POKE(wantAlt?MIDI_FIFO_ALT:MIDI_FIFO, 0xB0 | chan); // control change message
	POKE(wantAlt?MIDI_FIFO_ALT:MIDI_FIFO, 0x7B); // all notes off command
	POKE(wantAlt?MIDI_FIFO_ALT:MIDI_FIFO, 0x00);
}
//shut all channels from 0 to 15
void midiShutAllChannels(bool wantAlt)
{
	uint8_t i=0;
	for(i=0;i<16;i++)
	{
	POKE(wantAlt?MIDI_FIFO_ALT:MIDI_FIFO, 0xB0 | i); // control change message
	POKE(wantAlt?MIDI_FIFO_ALT:MIDI_FIFO, 0x7B); // all notes off command
	POKE(wantAlt?MIDI_FIFO_ALT:MIDI_FIFO, 0x00);
	}
}

//Stop all ongoing sounds (ie MIDI panic button)
void midiShutUp(bool wantAlt)
	{
		POKE(wantAlt?MIDI_FIFO_ALT:MIDI_FIFO, 0xFF);
	}

//Reset all instruments for all 16 channels to acoustic grand piano 00
void resetInstruments(bool wantAlt)
	{
		uint8_t i;
		for(i=0;i<16;i++)
		{
			POKE(wantAlt?MIDI_FIFO_ALT:MIDI_FIFO, 0xC0 | i);
			POKE(wantAlt?MIDI_FIFO_ALT:MIDI_FIFO, 0x00);
		}
		midiShutUp(wantAlt);
	}
//Set an instrument prg to channel chan
void prgChange(uint8_t prg, uint8_t chan, bool wantAlt)
	{
		POKE(wantAlt?MIDI_FIFO_ALT:MIDI_FIFO, 0xC0 | chan);
		POKE(wantAlt?MIDI_FIFO_ALT:MIDI_FIFO, prg);
	}
//Cuts an ongoing note on a specified channel
void midiNoteOff(uint8_t channel, uint8_t note, uint8_t speed, bool wantAlt)
	{
		POKE(wantAlt?MIDI_FIFO_ALT:MIDI_FIFO, 0x80 | channel);
		POKE(wantAlt?MIDI_FIFO_ALT:MIDI_FIFO, note);
		POKE(wantAlt?MIDI_FIFO_ALT:MIDI_FIFO, speed);
	}
//Plays a note on a specified channel
void midiNoteOn(uint8_t channel, uint8_t note, uint8_t speed, bool wantAlt)
	{
		POKE(wantAlt?MIDI_FIFO_ALT:MIDI_FIFO, 0x90 | channel);
		POKE(wantAlt?MIDI_FIFO_ALT:MIDI_FIFO, note);
		POKE(wantAlt?MIDI_FIFO_ALT:MIDI_FIFO, speed);
	}
	
//this is a small code to enable the VS1053b's midi mode; present on the Jr2 and K2. It is necessary for the first revs of these 2 machines
void initVS1053MIDI(void) {
    uint8_t n;
    uint16_t addr, val, i=0;

  while (i<sizeof(plugin)/sizeof(plugin[0])) {
    addr = plugin[i++];
    n = plugin[i++];
    if (n & 0x8000) { /* RLE run, replicate n samples */
      n &= 0x7FFF;
      val = plugin[i++];
      while (n--) {
        //WriteVS10xxRegister(addr, val);
        POKE(VS_SCI_ADDR,addr);
        POKEW(VS_SCI_DATA,val);
        POKE(VS_SCI_CTRL,1);
        POKE(VS_SCI_CTRL,0);
		while (PEEK(VS_SCI_CTRL) & 0x80);
      }
    } else {           /* Copy run, copy n samples */
      while (n--) {
        val = plugin[i++];
        //WriteVS10xxRegister(addr, val);
        POKE(VS_SCI_ADDR,addr);
        POKEW(VS_SCI_DATA,val);
        POKE(VS_SCI_CTRL,1);
        POKE(VS_SCI_CTRL,0);
		while (PEEK(VS_SCI_CTRL) & 0x80);
      }
    }
  }
}


void initMidiRecord(struct midiRecord *rec)
{
	rec->totalDuration=0;
	rec->fileName = malloc(sizeof(char) * 64);
	rec->format = 0;
	rec->trackcount =0;
	rec->tick = 48;
	rec->fileSize = 0;
	rec->fudge = 25.1658;
	rec->nn=4;
	rec->dd=2;
	rec->cc=24;
	rec->bb=8;
	rec->currentSec=0;
	rec->totalSec=0;
}

void initBigList(struct bigParsedEventList *list)
{
	list->hasBeenUsed = false;
	list->trackcount = 0;
	list->TrackEventList = (aTOEPtr)NULL;
}

//gets a count of the total MIDI events that are relevant and left to play
uint32_t getTotalLeft(struct bigParsedEventList *list)
	{
	uint32_t sum=0;
	uint16_t i=0;

	for(i=0; i<list->trackcount; i++)
		{
		sum+=list->TrackEventList[i].eventcount;
		}
	return sum;
	}
	
