#ifndef MUVGMPLAY_H
#define MUVGMPLAY_H

FILE *load_VGM_file(char *);
uint8_t checkVGMHeader(FILE *);
int8_t playback(FILE *);

extern uint32_t tooBigWait;
extern bool comeRightTrough;

#endif // MUVGMPLAY_H