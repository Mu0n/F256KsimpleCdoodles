#ifndef MUMIDI_H
#define MUMIDI_H
//SAM2695 midi
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

void resetInstruments(bool wantAlt);
void midiShutAChannel(uint8_t chan, bool wantAlt);
void midiShutUp(bool wantAlt);
void prgChange(uint8_t prg, uint8_t chan, bool wantAlt);
void midiNoteOff(uint8_t chan, uint8_t note, uint8_t speed, bool wantAlt);
void midiNoteOn(uint8_t chan, uint8_t note, uint8_t speed, bool wantAlt);
void initVS1053MIDI(void);
void emptyFIFO_ALT(void);

#endif // MUMIDI_H