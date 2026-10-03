#ifndef MUMIDIPLAY2_H
#define MUMIDIPLAY2_H

typedef struct MIDITrackParser {
	uint32_t length, offset, start;
	uint32_t delta;
	uint8_t cmd[6];
	uint8_t lastCmd;
	bool is2B;
	bool isDone;
} MIDTrackP;

typedef struct MIDIParser {
	uint16_t nbTracks;
	uint16_t ticks;
	uint32_t timer0PerTick;
	uint32_t progTime;
	bool isWaiting;
	uint32_t cuedDelta;
	uint16_t cuedIndex;
	uint16_t isMasterDone;
	struct MIDITrackParser *tracks;
} MIDP;


uint8_t PEEK24(uint32_t addr);
uint8_t POKE24(uint32_t addr, uint8_t value);

void detectStructure(uint16_t, struct midiRecord *);
uint8_t loadSMFile(char *, uint32_t);
//uint16_t sendSyxFile(uint32_t);
int16_t sendSyxStream(char *name);
uint8_t FAR8_loadSMFile(char *, uint32_t);
uint16_t readBigEndian16(uint32_t);
uint32_t readBigEndian32(uint32_t);
uint32_t divide32by16(uint32_t, uint16_t);
uint32_t mul32x32_low32(uint32_t, uint32_t);
bool hasSyxExt(const char *);
size_t my_strlen(const char *);

uint8_t readMIDIEvent(uint8_t);
uint32_t readDelta(uint8_t);
uint8_t readMIDICmd(uint8_t);
uint8_t skipWhenFFCmd(uint8_t, uint8_t, uint8_t);
void chainEvent(uint8_t);
void performMIDICmd(uint8_t);
void exhaustZeroes(uint8_t);
void playMidi(void);

void initTrack(uint32_t);
void destroyTrack(void);
void playMidi(void);
uint8_t readMIDIEvent(uint8_t);
void sniffNextMIDI(void);
uint32_t shift_add_mul(uint32_t, uint32_t);
void sendAME(uint8_t, uint8_t, uint8_t, uint8_t, bool);
void armTimer0(void);

extern struct MIDIParser theOne;
extern bool midiChip;
extern bool shimmerChanged[16];
extern uint8_t shimmerBuffer[16];
extern uint16_t disp[16];
extern uint8_t vals[16];

#endif // MUMIDIPLAY2_H