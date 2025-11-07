#ifndef TEXTUI_H
#define TEXTUI_H

#include "f256lib.h"

typedef struct SidField {
	uint8_t value;
	uint8_t *sidIPtr; //pointer to the sid instrument's exact location
	bool isHighNib; //is the high nibble, otherwise it's the low one
	bool is_dirty; //needs to be updated to the chip and sidInstrument
	uint8_t tX, tY; //text location on screen for the text UI
} sF;

void printInstrumentHeaders(void);
void updateValues(void);
void initSIDFields(void);
void fieldToChip(uint8_t);
void init_sid_field(struct SidField *, uint8_t *, bool, uint8_t, uint8_t, uint8_t);
void updateHighlight(uint8_t, uint8_t);
void randomInst(void);

extern struct SidField sid_fields[];
extern uint8_t indexUI;
extern uint8_t navWSJumps[];

#endif // TEXTUI_H