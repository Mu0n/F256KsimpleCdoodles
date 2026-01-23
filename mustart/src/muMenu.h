#ifndef MUMENU_H
#define MUMENU_H

#include "f256lib.h"

#define MAXNAME 20
#define MAXDESC 35 
#define MAXFILE 40

#define MAXARGS 4
#define MAXITEMS 12
#define NBCATS 4
#define MENUTOPX 7
#define MENUTOPY 10
#define GAPX 20


//keeps track of tempo changes and pre-calculations of usPerTick and usPerTimer0 to make it lighter during parsing
typedef struct menuItem{
	char name[MAXNAME];
	char desc[MAXDESC];
	char file[MAXFILE];
} mIt;

typedef struct menuCatList{
	struct menuItem items[MAXITEMS];
	uint8_t fillIndex; //starts at 0 and fills up as elements of a categories are found during initial read
} mCL;

void goLeftOrRight(bool, uint8_t *, uint8_t *);
void goUpOrDown(bool, uint8_t *, uint8_t);
void initItems(void);
void readMenuContent(void);
void displayMenu(uint8_t, uint8_t, uint8_t);
void displayOneItem(uint8_t, uint8_t, uint8_t, uint8_t);
void displayCats(uint8_t, uint8_t, uint8_t);
int readLine(FILE *);
void updateSoundStatus(uint8_t *);
extern struct menuCatList cats[];
extern uint8_t currentCat;
extern uint8_t chipActive;

#endif // MUMENU_H