#ifndef MODUUI_H
#define MODUUI_H

//Offsets from SPR_BASE in the sprite sheet
#define UI_ORA_NB 16*64
#define UI_DIALS  32*64
#define UI_SLIDS  40*64
#define UI_SWTCH  56*64
#define UI_STAT   58*64
#define UI_ACTIV  64
#define UI_ROW    8*64

#include "f256lib.h"

typedef struct generic_UI
{
	uint8_t s;
	int16_t x,y;
	bool isHovered;
	bool isClicked;
	bool isDisabled;
	uint32_t addr; //base sprite gfx address
	uint8_t parentIndex; //which index it can be found in the parent array
	uint8_t size; //8, 16 or 32x32
	uint8_t x1, x2, y1, y2; //internal clickable area to scan
	uint8_t actionID; //used by your piece of code to drive their behavior in a switch case
} generic_UI;

typedef struct slider_UI
{
	struct generic_UI gen;
	
	uint8_t value8,min8,max8;
	uint16_t value16,min16,max16;
} slider_UI;

typedef struct dial_UI
{
	struct generic_UI gen;
	
	uint8_t value8,min8,max8;
	uint16_t value16,min16,max16;
} dial_UI;

typedef struct radioB_UI
{
	struct generic_UI gen;
	bool isGroupExclusive; //if one is activated, the rest of exclusives must deactivate
	uint8_t groupID;
} radioB_UI;

uint8_t getEigth(uint16_t, uint16_t, uint16_t);
void setGeneric(uint8_t, uint16_t, uint16_t, uint32_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, struct generic_UI *);
void updateGeneric(struct generic_UI *, uint8_t, uint32_t);

void showGeneric(struct generic_UI *);
void setRadioB(struct radioB_UI *, bool, uint8_t, bool);
void updateRadioB(struct radioB_UI *);

void updateSlider(struct slider_UI *, uint32_t);
void setSlider(struct slider_UI *, uint8_t, uint8_t, uint8_t, uint16_t, uint16_t, uint16_t, uint32_t);

void updateDial(struct dial_UI *, uint32_t);
void setDial(struct dial_UI *, uint8_t, uint8_t, uint8_t, uint16_t, uint16_t, uint16_t, uint32_t);
#endif // MODUUI_H
