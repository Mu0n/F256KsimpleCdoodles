#ifndef TEXTUI_H
#define TEXTUI_H

#include "f256lib.h"

#define INSTR_LINE 28
#define START_X_VIS       4 //start of channel output x on screen when snooping
#define START_HEIGHT_VIS  5 //start of channel output  yon screen when snooping
#define START_HEIGHT_VIS_ALT  25 //start of channel output  yon screen when snooping

void instructions(void);
void textUI(void);
void eraseLine(uint8_t);
void axes_info(uint8_t);
void show_inst(uint8_t);
void show_all_inst(void);
void wipe_inst(void);

extern uint8_t playChan;

#endif // TEXTUI_H