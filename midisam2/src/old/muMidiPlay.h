#ifndef MUMIDIPLAY_H
#define MUMIDIPLAY_H

#include "f256lib.h"
#include "../src/muMidi.h"
#include "../src/timer0.h"
#include <string.h>
#include "../src/muUtils.h"  //contains basic MIDI functions I often use


//PROTOTYPES
void adjustOffsets(struct bigParsedEventList *);	

uint8_t readDigestFile(char *, struct midiRecord *, struct bigParsedEventList *);
void playEmbeddedDim(uint32_t);

uint8_t playmiditype0(struct midiRecord *, struct bigParsedEventList *); 
uint8_t playmidi(struct midiRecord *, struct bigParsedEventList *);

//uint32_t getTotalLeft(struct bigParsedEventList *);
void sendAME(aMEPtr, bool);

uint8_t checkKeySkip(void);

#endif // MUMIDIPLAY_H
