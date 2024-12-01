#include "D:\F256\llvm-mos\code\midiTest\.builddir\trampoline.h"

#include "f256lib.h"
#include "../src/muMidi.h"

void midiShutAChannel(uint8_t chan)
{
	POKE(MIDI_FIFO, 0xB0 | chan); // control change message
	POKE(MIDI_FIFO, 0x7B); // all notes off command
	POKE(MIDI_FIFO, 000);
}
//Stop all ongoing sounds (ie MIDI panic button)
void midiShutUp()
	{
		POKE(MIDI_FIFO, 0xFF);
	}

//Reset all instruments for all 16 channels to ac. grand piano 00
void resetInstruments()
	{
		uint8_t i;
		for(i=0;i<16;i++)
		{
			POKE(MIDI_FIFO, 0xC0 | i);
			POKE(MIDI_FIFO, 0x00);
		}
		midiShutUp();
	}
//Set an instrument prg to channel chan
void prgChange(uint8_t prg, uint8_t chan)
	{
		POKE(MIDI_FIFO, 0xC0 | chan);
		POKE(MIDI_FIFO, prg);
	}
//Cuts an ongoing note on a specified channel
void midiNoteOff(uint8_t channel, uint8_t note, uint8_t speed)
	{
		POKE(MIDI_FIFO, 0x80 | channel);
		POKE(MIDI_FIFO, note);
		POKE(MIDI_FIFO, 0x4F);
	}
//Plays a note on a specified channel
void midiNoteOn(uint8_t channel, uint8_t note, uint8_t speed)
	{
		POKE(MIDI_FIFO, 0x90 | channel);
		POKE(MIDI_FIFO, note);
		POKE(MIDI_FIFO, speed);
	}
