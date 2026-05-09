#ifndef MUMODPLAY_H
#define MUMODPLAY_H

#define MOD_BODY 0x180000

void copyToRAM(FILE *);
FILE *load_MOD_file(char *);
void checkMODHeader(FILE *);
int8_t playbackMOD(FILE *, bool, bool);



extern uint32_t tooBigWait;
extern bool comeRightTrough;
extern uint32_t needle;
extern uint32_t totalWait;
extern uint32_t loopBackTo;
extern uint32_t samplesSoFar;
extern uint16_t readDebug;
extern bool iRT;
extern bool oneLoop;

#endif // MUMODPLAY_H