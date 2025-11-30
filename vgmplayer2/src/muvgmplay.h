#ifndef MUVGMPLAY_H
#define MUVGMPLAY_H

#define VGM_BODY 0x100000

void copyToRAM(FILE *);
FILE *load_VGM_file(char *);
void checkVGMHeader(FILE *);
int8_t playback(FILE *, bool, bool);
uint8_t getStart(uint16_t);



extern uint32_t tooBigWait;
extern bool comeRightTrough;
extern uint32_t needle;
extern uint32_t totalWait;
extern uint32_t loopBackTo;
extern uint32_t samplesSoFar;
extern uint16_t readDebug;
extern bool iRT;

#endif // MUVGMPLAY_H