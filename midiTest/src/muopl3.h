#ifndef MUOPL3_H
#define MUOPL3_H

#include "f256lib.h"

#define OPL_ADDR_L   0xD580   //Address pointer for ports 0x000 - 0x0FF
#define OPL_DATA     0xD581   //Data register for all ports
#define OPL_ADDR_H   0xD582   //Address pointer for ports 0x100 - 0x1FF

//registers internal to the YMF262 chip
//nomenclature used in adlib tracker ii and various datasheets for the YMF262

//Scope: chip
#define OPL_EN          0x01     //Chip wide enable register. set to 0x20 to start using the chip
#define OPL_T1          0x02     //Chip wide timer 1
#define OPL_T2          0x03     //Chip wide timer 2
#define OPL_IRQ         0x04     //Chip wide IRQ reset, mask, start
#define OPL_FOE         0x104    //Chip wide four operator enable OPL3 only, select modes: 4op BE, 4OP AD, 4OP 9C, 4OP 1-4, 4OP 0-3
#define OPL_OPL3        0x105    //Chip wide OPL3 only, set to 0x01 to enable opl3 features
#define OPL_CSW         0x08     //Chip wide composite sine wave mode (always unused), note select in bit6
#define OPL_PERC        0xBD     //Chip wide perc mode, bass, snare, tom, cymb, hihat on

//Scope: channel
#define OPL_CH_F_LO     0xA0     //Base address of channel frequency num low byte
#define OPL_CH_KBF_HI   0xB0     //Base address of keyon, block, freq num hi byte
#define OPL_CH_FEED     0xC0     //Base address of panning, feedback, synth type

//Scope: operator
#define OPL_OP_TVSKF    0x20     //Base address of Tremolo, vibrator, sustain, KSR, F multimap
#define OPL_OP_KSLVOL   0x40     //Base address of key scale level, output volume
#define OPL_OP_AD       0x60     //Base address of attack, decay
#define OPL_OP_SR       0x80     //Base address of sustain, release
#define OPL_OP_WAV      0xE0     //Base address of waveform select

//the following three structs are for midi event playback. keep every aMIDIEvent's content in far memory
typedef struct opl3Instrument {
    uint8_t chan;
	uint8_t OP1_TVSKF, OP2_TVSKF;
	uint8_t OP1_KSLVOL, OP2_KSLVOL;
	uint8_t OP1_AD, OP2_AD;
	uint8_t OP1_SR, OP2_SR;
	uint8_t OP1_WAV, OP2_WAV;
   } opl3I;
		
void opl3_initialize(void);
void opl3_write(uint16_t, uint8_t);
void opl3_note(uint8_t, uint16_t, uint8_t, bool);
void opl3_setInstrument(struct opl3Instrument);

extern const uint16_t opl3_fnums[12];

#endif // MUOPL3_H