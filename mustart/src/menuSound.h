#ifndef MENUSOUND_H
#define MENUSOUND_H

#include "f256lib.h"

#define TIMER_FRAMES 0
#define TIMER_SECONDS 1
#define TIMER_MENUSOUND_COOKIE 1
#define TIMER_MENUSOUND_DELAY 2

#define PSG_SYS1  0xD6A1 //bit2 if 0, both mixed in mono. if 1, separate left and right speakers
#define PSG_LEFT  0xD600
#define PSG_RIGHT 0xD610
#define PSG_BOTH  0xD608

void killSound(void);
void midiShutAllChannels(void);
void setStereoPSG(void);
void shutPSG(void);
void psgNoteOn(uint8_t, uint16_t, uint8_t, uint8_t,uint8_t);
void psgNoteOff(uint8_t, uint16_t);
void initMenuSoundTimer(void);
void relaunchTimer(uint8_t);

extern struct timer_t menuSoundTimer;
#endif // MENUSOUND_H