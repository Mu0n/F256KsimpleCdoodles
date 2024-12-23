#define F256LIB_IMPLEMENTATION



// VS1053 - MP3 Playback
#define cVS1053_CTRL_REG            (*(volatile __far uint8_t *) 0xF01700 ) // 0xF01700 - RW - VS1053Q
#define cVS1053_CTRL_Start          0x01
#define cVS1053_CTRL_RWn            0x02
#define cVS1053_CTRL_Busy           0x08
#define cVS1053_CTRL_ADDY           (*(volatile __far uint8_t *) 0xF01701 ) // 0xF01701 - RW - Register Access Addy
#define cVS1053_CTRL_DATA           (*(volatile __far uint16_t *) 0xF01702 ) // 0xF01702 - RW - Register Access Data
#define cVS1053_FIFO_COUNT          (*(volatile __far uint16_t *) 0xF01704 ) // 0xF01704 - Read - FIFO Count ({Bit[7] = FIFO_EMPTY (when 1), Bit[6] = FIFO_FULL (When 1), Bit[5:3] == 000, FIFO_COUNT[10:8], FIFO_COUNT[7:0])
#define cVS1053_FIFO_full           0x4000
#define cVS1053_FIFO_Empty          0x8000
#define cVS1053_NOTHING_REG         (*(volatile __far uint8_t *) 0xF01706 ) // 0xF01706 - Nothing Here
#define cVS1053_STREAM_DATA         (*(volatile __far uint8_t *) 0xF01707 ) // 0xF01707 - Write


#include "f256lib.h"


int main(int argc, char *argv[]) {
	
return 0;}

