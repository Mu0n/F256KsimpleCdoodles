#ifndef MUTEXTUI_H
#define MUTEXTUI_H

#include "f256lib.h"
#include "../src/muMidi.h"  //contains basic MIDI functions I often use

#define textColorOrange 0x09
#define textColorWhite  0x0F
#define textColorBlack  0x00
#define textColorYellow 0x0D
#define textColorLGreen 0x0E
#define textColorGray   0x05
#define textColorLightBlue 0x07
#define DIR_NEXT_COL 29

#define INST_NAME_X 6
#define MENU_Y 29
#define MIDI_INST_NAME 0x20000


typedef struct filePick {
	uint8_t fileCount;
	uint8_t choice; //selection number for the directory browser. is used for placing the arrow character left of a filename
}fiPi;

void modalHelp(const char **, uint16_t);
void eraseModalHelp(uint16_t);
void directory(uint16_t, uint8_t, struct filePick *);

void displayInfo(struct midiRecord *);
void extraInfo(struct midiRecord *,struct bigParsedEventList *);
void superExtraInfo(struct midiRecord *, uint8_t);
void updateInstrumentDisplay(uint8_t, uint8_t);


#endif // MUTEXTUI_H