#ifndef MUMIDIPLAY_H
#define MUMIDIPLAY_H

#include "f256lib.h"
#include "../src/muMidi.h"
#include "../src/timer0.h"
#include <string.h>
#include "../src/muUtils.h"  //contains basic MIDI functions I often use


//PROTOTYPES
uint8_t loadSMFile(char *, uint32_t);
int16_t getAndAnalyzeMIDI(struct midiRecord *, struct bigParsedEventList *);
void detectStructure(uint16_t, struct midiRecord *, struct bigParsedEventList *);
int16_t findPositionOfHeader(uint32_t);
void adjustOffsets(struct bigParsedEventList *);	
int8_t parse(uint16_t, bool, struct midiRecord *, struct bigParsedEventList *);

uint8_t writeDigestFile(char *, struct midiRecord *, struct bigParsedEventList *);
uint8_t readDigestFile(char *, struct midiRecord *, struct bigParsedEventList *);

uint8_t playmiditype0(struct midiRecord *, struct bigParsedEventList *); 
uint8_t playmidi(struct midiRecord *, struct bigParsedEventList *);

//uint32_t getTotalLeft(struct bigParsedEventList *);
void sendAME(aMEPtr, bool);

#endif // MUMIDIPLAY_H
