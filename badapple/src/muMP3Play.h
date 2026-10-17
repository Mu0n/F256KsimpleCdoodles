#ifndef MUMP3PLAY_H
#define MUMP3PLAY_H

#include "f256lib.h"

#define CHUNK32B 0x20
#define CHUNK64B 0x40
#define CHUNK128B 0x80
#define CHUNK2K  0x0800
#define CHUNK8K  0x2000
#define SILENTBYTE        0x80
#define BASE_MP3_BUFFER 0x100000

uint8_t prepMP3ForPlay(char *);
bool isMP3Done(void);
void rewindAndPlayMP3(void);
int8_t MP3LoopPass(void);
void read2KChunk(void);
void silencePad(void);
void initialInjectMP3(void);
void closeDownMp3File(void);


#endif //MUMP3PLAY_H