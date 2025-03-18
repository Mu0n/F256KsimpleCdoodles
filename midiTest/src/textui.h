#ifndef TEXTUI_H
#define TEXTUI_H

#include "../src/globals.h"

#define textColorGreen  0x04
#define textColorOrange 0x09
#define textColorRed    0x91
#define textColorBlue   0x07
#define textColorGray   0x05


void refreshInstrumentText(struct glTh*);
void channelTextMenu(struct glTh*);
void refreshBeatTextChoice(struct glTh*);
void beatsTextMenu(struct glTh *);
void chipSelectTextMenu(struct glTh *);
void textTitle(struct glTh *);
void updateTempoText(uint8_t);
void showMIDIChoiceText(struct glTh*);
void showChipChoiceText(struct glTh*);
void instListShow(struct glTh*);
void highLightInstChoice(bool, struct glTh*);
void modalMoveUp(struct glTh*, bool);
void modalMoveDown(struct glTh*, bool);
void modalMoveLeft(struct glTh*);
void modalMoveRight(struct glTh*);

#endif // TEXTUI_H