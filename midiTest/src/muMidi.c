#include "f256lib.h"
#include "../src/muMidi.h"

const uint16_t plugin[28] = { /* Compressed plugin */
  0x0007, 0x0001, 0x8050, 0x0006, 0x0014, 0x0030, 0x0715, 0xb080, /*    0 */
  0x3400, 0x0007, 0x9255, 0x3d00, 0x0024, 0x0030, 0x0295, 0x6890, /*    8 */
  0x3400, 0x0030, 0x0495, 0x3d00, 0x0024, 0x2908, 0x4d40, 0x0030, /*   10 */
  0x0200, 0x000a, 0x0001, 0x0050
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

void emptyFIFO_ALT(void)
{
	uint16_t toDo, i;
	
	if(!((PEEK(MIDI_CTRL_ALT))&0x02))
	{
		toDo = PEEKW(MIDI_RXD_ALT) & 0x0FFF;
		for(i=0;i<toDo;i++) PEEK(MIDI_FIFO_ALT);
	}
	
	if(!((PEEK(MIDI_CTRL))&0x02))
	{
		toDo = PEEKW(MIDI_RXD) & 0x0FFF;
		for(i=0;i<toDo;i++) PEEK(MIDI_FIFO);
	}
}

void midiShutAChannel(uint8_t chan, bool wantAlt)
{
	POKE(wantAlt?MIDI_FIFO_ALT:MIDI_FIFO, 0xB0 | chan); // control change message
	POKE(wantAlt?MIDI_FIFO_ALT:MIDI_FIFO, 0x7B); // all notes off command
	POKE(wantAlt?MIDI_FIFO_ALT:MIDI_FIFO, 0x00); 
}
//Stop all ongoing sounds (ie MIDI panic button)
void midiShutUp(bool wantAlt)
	{
		POKE(wantAlt?MIDI_FIFO_ALT:MIDI_FIFO, 0xFF);
	}

//Reset all instruments for all 16 channels to ac. grand piano 00
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
		printf("noteone %d %04x",wantAlt,wantAlt?MIDI_FIFO_ALT:MIDI_FIFO);
		POKE(wantAlt?MIDI_FIFO_ALT:MIDI_FIFO, 0x90 | channel);
		POKE(wantAlt?MIDI_FIFO_ALT:MIDI_FIFO, note);
		POKE(wantAlt?MIDI_FIFO_ALT:MIDI_FIFO, speed);
	}
}