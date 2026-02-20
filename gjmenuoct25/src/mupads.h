#ifndef MUPADS_H
#define MUPADS_H

#define PAD_CTRL    0xD880
#define PAD_STAT    0xD880
//D880 settings
//        7  6  5  4 |  3  2   1  0
// PAD_TRIG XX XX XX | XX MODE XX NES_EN

#define CTRL_TRIG     0b10000000 // use this to launch a trigger of the polling. 
#define CTRL_TRIG_OFF 0b00000000 // clears the trigger
#define STAT_DONE     0b01000000     //use this to verify the polling is done and the 

//states are ready to be read



//use either of these with a bitwise 'or' with the previous 3 lines to select NES or SNES modes
#define CTRL_MODE_NES  0b00000001
#define CTRL_MODE_SNES 0b00000101

#define PAD0    0xD884
#define PAD0_S   0xD885 //snes only
#define PAD1    0xD886
#define PAD1_S   0xD887 //snes only
#define PAD2    0xD888
#define PAD2_S   0xD889 //snes only
#define PAD3    0xD88A
#define PAD3_S   0xD88B //snes only

//Poll the pad, do a bitwise 'And' and if the value is 0, then it's pressed

#define NES_A      0x7F
#define NES_B      0xBF
#define NES_SELECT 0xDF
#define NES_START  0xEF
#define NES_UP     0xF7
#define NES_DOWN   0xFB
#define NES_LEFT   0xFD
#define NES_RIGHT  0xFE

//Poll the pad, do a bitwise 'And' and if the value is 0, then it's pressed
#define SNES_B      0x80
#define SNES_Y      0x40
#define SNES_SELECT 0x20
#define SNES_START  0x10
#define SNES_UP     0x08
#define SNES_DOWN   0x04
#define SNES_LEFT   0x02
#define SNES_RIGHT  0x01

#define SNES_A      0x08
#define SNES_X      0x04
#define SNES_L      0x02
#define SNES_R      0x01

#include "f256lib.h"

void pollNES(void);
void pollSNES(void);
bool padPollIsReady(void);
void padPollDelayUntilReady(void);

#endif // MUPADS_H