#ifndef MIDIBOUNCERGFX_H
#define MIDIBOUNCERGFX_H

#define FREAKOUT_INIT 10 //how many frames of freakout needs to be done
#define SPR_MIDI_PAL  0x25000
#define SPR_MIDI   0x24C00
#define SPR_OFF_x  2 //leftmost x coord having a lit pixel
#define SPR_OFF_y  9 //topmost y coord having a lit pixel
#define SPR_WIDTH  28 //width
#define SPR_HEIGHT 13 //height
#define OFF_RIGHT (320-2) //rightmost border from the right edge of screen
#define OFF_TOP   30 //topmost border from the top edge of the screen
#define AREA_WIDTH 180 //width of bouncy area
#define AREA_HEIGHT 160 //height of bouncy area
#define OFF_LEFT (OFF_RIGHT - AREA_WIDTH) //leftmost border from the left edge of screen
#define OFF_BOTTOM (OFF_TOP + AREA_HEIGHT)
#define SPR_MAX_X  (OFF_RIGHT - 32 + SPR_OFF_x)
#define SPR_MIN_X  (OFF_LEFT - SPR_OFF_x)

#define SPR_MIN_Y  (OFF_TOP - SPR_OFF_y + 2)
#define SPR_MAX_Y  (OFF_BOTTOM - 32 + SPR_OFF_y)

void drawBouncingBox(void);
void initMIDISprite(void);
void updateMIDISprite(void);

extern uint16_t midiSpr_x, midiSpr_y;
extern int8_t midiSpr_vx, midiSpr_vy;
extern uint8_t freakoutCoundDown;
extern bool isFreakingOut;

#endif // MIDIBOUNCERGFX_H
