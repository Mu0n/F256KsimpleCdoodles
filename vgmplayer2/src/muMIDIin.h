#ifndef MUMIDIIN_H
#define MUMIDIIN_H

#include "f256lib.h"


typedef struct {
uint8_t recByte; 	//last received midi in byte
uint8_t nextIsNote; //received 1 midi byte, 1= awaiting midi note one
uint8_t nextIsSpeed; //received 2 midi bytes, 1= awaiting last one for velocity
uint8_t isHit; //1= will have to deal with note on event; 0 = will have to deal with note off
uint8_t lastCmd; //keeps track of last cmd for run-on MIDI scheme
uint8_t lastNote; //keeps track of midi note value that was used last time
uint8_t storedNote; //keeps track of midi note for run-on MIDI scheme, when the command is chopped off because we repeat the last command if the first byte is less than 0x80
} midiInData;


void resetMID(midiInData *);
void dealMIDIIn(midiInData *);
extern bool noteColors[];

#endif // MUMIDIIN_H