#ifndef MUMIDI_H
#define MUMIDI_H

#define MIDI_FIFO 	   0xDDA1
#include "f256lib.h"

void resetInstruments(void);
void midiShutAChannel(uint8_t chan);
void midiShutUp(void);
void prgChange(uint8_t prg, uint8_t chan);
void midiNoteOff(uint8_t chan, uint8_t note, uint8_t speed);
void midiNoteOn(uint8_t chan, uint8_t note, uint8_t speed);

#endif // MUMIDI_H