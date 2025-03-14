#include "D:\F256\llvm-mos\code\opl3\.builddir\trampoline.h"

#include "f256lib.h"
#include "../src/muopl3.h"

const uint16_t opl3_fnums[] = {0x205, 0x223, 0x244, 0x267, 0x28B, 0x2B2,
						       0x2DB, 0x306, 0x334, 0x365, 0x399, 0x3CF};
	
void opl3_initialize() {
    // Operator settings (e.g., Operator 1, Channel 0)
    opl3_write(0x20, 0x01); // Enable the modulator
    opl3_write(0x40, 0xFF); // Set the modulator's output level
    opl3_write(0x60, 0xFF); // Set attack/release times
    opl3_write(0x80, 0xFF); // Set sustain/decay times
    opl3_write(0xE0, 0x00);
	
    opl3_write(0x23, 0x01);
    opl3_write(0x43, 0x00);
    opl3_write(0x63, 0xF5);
    opl3_write(0x83, 0x75);
    opl3_write(0xE3, 0x00);
	opl3_write(0xC0, 0x3C); // Set output channel
}


// Function to write a value to a YMF262 register
void opl3_write(uint16_t address, uint8_t value) {
    if (address < 0x100) {
        // Address in range 0x000 - 0x0FF
		POKE(OPL_ADDR_L, address);
	
    } else {
        // Address in range 0x100 - 0x1FF
		POKE(OPL_ADDR_H,(address & 0xFF));
    }
    // Write the value to OPL_DATA
	POKE(OPL_DATA, value);
}


// Function to play a note. Pick one of 12 tones from opl3_fnums, pick your 0-7 octave 'block' and duration in frames
void opl3_note(uint8_t channel, uint16_t fnum, uint8_t block, bool onOrOff) {
    // Set frequency (low byte)
    opl3_write(0xA0 + channel, fnum & 0xFF);
    // Set block/frequency (high byte) and enable sound (Key-On) or off depending on onOrOff value
    opl3_write(0xB0 + channel, ((fnum >> 8) & 0x03) | (block << 2) | (onOrOff?0x20:0x00));

}

