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

#define MENU_Y 29


typedef struct filePick {
	uint8_t fileCount;
	uint8_t choice; //selection number for the directory browser. is used for placing the arrow character left of a filename
}fiPi;

void directory(uint16_t, uint8_t, struct filePick *);

void displayInfo(struct midiRecord *);
void superExtraInfo(struct midiRecord *);
void updateInstrumentDisplay(uint8_t, uint8_t);
void cycleMuteSelection(int8_t);
void updateMuteDisplay(uint8_t);


#endif // MUTEXTUI_H