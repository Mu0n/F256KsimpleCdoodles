#ifndef MUFILEDIPLOMACY_H
#define MUFILEDIPLOMACY_H

#define KQ 16

void kqReset(void);
bool pullEventB(void);       // called from main.c: stash first, then the live queue
int16_t fileReadDiplomat(void *, uint16_t, uint16_t, uint8_t *);
int16_t kernelReadDiplomat(uint8_t fd, void *buf, uint16_t nbytes);
bool pullEvent(void);
extern uint8_t kqHead, kqTail;

#endif //MUFILEDIPLOMACY_H