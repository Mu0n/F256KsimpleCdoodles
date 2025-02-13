#include "f256lib.h"
#include "../src/muMidi.h"



const uint16_t plugin[28] = { /* Compressed plugin  for the VS1053b to enable real time midi mode */
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
	
}