#ifndef MUSPRITE_H
#define MUSPRITE_H


#define SPR_BOY1_BASE    0x10400 //sprite 0
#define SPR_THNG_BASE    0x10800 //sprite 1
#define SPR_GLR1_BASE    0x10C00 //sprite 2
#define SPR_CATH_BASE	 0x11000 //sprite 3
#define SPR_CAT_BASE	 0x11400 //sprite 4
#define SPR_SPR5_BASE	 0x11800 //sprite 5 todo
#define SPR_SPR6_BASE	 0x11C00 //sprite 6 todo
#define SPR_SPR7_BASE	 0x12000 //sprite 7 todo
#define SPR_SPR8_BASE	 0x12400 //sprite 8 todo
#define SPR_SPR9_BASE	 0x12800 //sprite 9 todo
#define SPR_SPRA_BASE	 0x12C00 //sprite A todo

#define SPR_DASH_BASE    0x13000

#define SPR_COUNT 10
#define SPR_STATE_COUNT 6
#define SPR_STATE_IDLE_R 0
#define SPR_STATE_IDLE_L 1
#define SPR_STATE_WALK_R 2
#define SPR_STATE_WALK_L 3
#define SPR_STATE_DASH_R 4
#define SPR_STATE_DASH_L 5

#define SPR_SIZE  16
#define SPR_SIZE_SQUARED  0x100


#define VKY_SP0_CTRL     0xD900 //Sprite #0’s control register
#define VKY_SP0_AD_L     0xD901 // Sprite #0’s pixel data address register
#define VKY_SP0_AD_M     0xD902
#define VKY_SP0_AD_H     0xD903
#define VKY_SP0_POS_X_L  0xD904 // Sprite #0’s X position register
#define VKY_SP0_POS_X_H  0xD905
#define VKY_SP0_POS_Y_L  0xD906 // Sprite #0’s Y position register
#define VKY_SP0_POS_Y_H  0xD907

#define VKY_SP1_CTRL     0xD908 //Sprite #1’s control register
#define VKY_SP1_AD_L     0xD909 // Sprite #1’s pixel data address register
#define VKY_SP1_AD_M     0xD90A
#define VKY_SP1_AD_H     0xD90B
#define VKY_SP1_POS_X_L  0xD90C // Sprite #1’s X position register
#define VKY_SP1_POS_X_H  0xD90D
#define VKY_SP1_POS_Y_L  0xD90E // Sprite #1’s Y position register
#define VKY_SP1_POS_Y_H  0xD90F

#define VKY_SP2_CTRL     0xD910 //Sprite #2’s control register
#define VKY_SP2_AD_L     0xD911 // Sprite #2’s pixel data address register
#define VKY_SP2_AD_M     0xD912
#define VKY_SP2_AD_H     0xD913
#define VKY_SP2_POS_X_L  0xD914 // Sprite #2’s X position register
#define VKY_SP2_POS_X_H  0xD915
#define VKY_SP2_POS_Y_L  0xD916 // Sprite #2’s Y position register
#define VKY_SP2_POS_Y_H  0xD917

#define VKY_SP3_CTRL     0xD918 //Sprite #3’s control register
#define VKY_SP3_AD_L     0xD919 // Sprite #3’s pixel data address register
#define VKY_SP3_AD_M     0xD91A
#define VKY_SP3_AD_H     0xD91B
#define VKY_SP3_POS_X_L  0xD91C // Sprite #3’s X position register
#define VKY_SP3_POS_X_H  0xD91D
#define VKY_SP3_POS_Y_L  0xD91E // Sprite #3’s Y position register
#define VKY_SP3_POS_Y_H  0xD91F


typedef struct sprStatus
{
	uint16_t x,y; //position
	bool rightOrLeft; //last facing
	bool isDashing;
	uint32_t addr; //base address
	uint8_t frame; //frame into view
	uint16_t sx, sy; //speed
	struct timer_t timer; //animation timer;
	uint8_t cookie; //cookie for timer
	uint8_t state; //which state: 0 idle, 1 walk right, 2 walk left, etc
	uint8_t *minIndexForState; //minimum index for given state
	uint8_t *maxIndexForState; //maximum index for given state
} sprStatus;

void mySpriteDefine(uint8_t, uint32_t, uint8_t, uint8_t, uint8_t, uint8_t, uint16_t, uint16_t, bool, uint8_t);
void mySetSpriteAddr(uint8_t, uint32_t);

extern sprStatus spriteStatuses[]; //hold characters sprite information
extern uint8_t stateDelays[]; //delays between each frame for each state
extern uint8_t dashDelay; //delay between dash frames

#endif // MUSPRITE_H