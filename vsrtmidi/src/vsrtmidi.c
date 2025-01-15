#define F256LIB_IMPLEMENTATION

#define MIDI_CTRL 	   0xDDA0
#define MIDI_FIFO 	   0xDDA1
#define MIDI_RXD 	   0xDDA2
#define MIDI_RXD_COUNT 0xDDA3
#define MIDI_TXD       0xDDA4
#define MIDI_TXD_COUNT 0xDDA5

//VS1053b midi
#define MIDI_CTRL_ALT 	    0xDDB0
#define MIDI_FIFO_ALT 	    0xDDB1
#define MIDI_RXD_ALT	    0xDDB2
#define MIDI_RXD_COUNT_ALT  0xDDB3
#define MIDI_TXD_ALT     	0xDDB4
#define MIDI_TXD_COUNT_ALT  0xDDB5


#include "f256lib.h"

const uint16_t plugin[28] = { /* Compressed plugin */
  0x0007, 0x0001, 0x8050, 0x0006, 0x0014, 0x0030, 0x0715, 0xb080, /*    0 */
  0x3400, 0x0007, 0x9255, 0x3d00, 0x0024, 0x0030, 0x0295, 0x6890, /*    8 */
  0x3400, 0x0030, 0x0495, 0x3d00, 0x0024, 0x2908, 0x4d40, 0x0030, /*   10 */
  0x0200, 0x000a, 0x0001, 0x0050
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
    if (n & 0x8000) { /* RLE run, replicate n samples */
      n &= 0x7FFF;
      val = plugin[i++];
      while (n--) {
        //WriteVS10xxRegister(addr, val);
        POKE(0xD701,addr);
        POKEW(0xD702,val);
        POKE(0xD700,1);
        POKE(0xD700,0);
		while (PEEK(0xd700) & 0x80);
      }
    } else {           /* Copy run, copy n samples */
      while (n--) {
        val = plugin[i++];
        //WriteVS10xxRegister(addr, val);
        POKE(0xD701,addr);
        POKEW(0xD702,val);
        POKE(0xD700,1);
        POKE(0xD700,0);
		while (PEEK(0xd700) & 0x80);
      }
    }
  }
}

int main(int argc, char *argv[]) {
	uint16_t howMany;
	uint8_t i, detected;
	uint8_t inst = 0;
	
	
		//codec enable all lines
	POKE(0xD620, 0x1F);
	POKE(0xD621, 0x2A);
	POKE(0xD622, 0x01);
	while(PEEK(0xD622) & 0x01);
	
	initVS1053MIDI();


	POKE(MIDI_FIFO_ALT,0xC0);
	POKE(MIDI_FIFO_ALT,30);
	
	
	while(true) 
        {
		if(!(PEEK(MIDI_CTRL) & 0x02)) //rx not empty
			{
				howMany= PEEKW(MIDI_RXD) & 0x0FFF; 
				for(i=0; i<howMany; i++)
				{
					detected = (uint8_t)PEEK(MIDI_FIFO);
					POKE(MIDI_FIFO_ALT ,detected);
				}
			}
					kernelNextEvent();
        if(kernelEventData.type == kernelEvent(key.PRESSED))
            {
			switch(kernelEventData.key.raw)
				{
					case 0xb6: //up arrow
						if(inst<127)
						{
							inst++;
							POKE(MIDI_FIFO_ALT,0xC0);
							POKE(MIDI_FIFO_ALT,inst);
							textGotoXY(0,0);printf("%s                  ",midi_instruments[inst]);
						}
						break;
					case 0xb7: //down arrow
						if(inst>0)
						{
							inst--;
							POKE(MIDI_FIFO_ALT,0xC0);
							POKE(MIDI_FIFO_ALT,inst);
							textGotoXY(0,0);printf("%s                   ",midi_instruments[inst]);
						}
						break;
				}
			}
			
		}
		

			
	return 0;}
}