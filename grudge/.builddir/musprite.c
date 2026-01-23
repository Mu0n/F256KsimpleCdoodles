#include "D:\F256\llvm-mos\code\grudge\.builddir\trampoline.h"

#include "f256lib.h"
#include "../src/musprite.h"

#define TIMER_FRAMES 0

sprStatus spriteStatuses[SPR_COUNT*2]; //hold characters sprite information


//states: 0=stand towards right, 1=stand towards left, 2 walk right, 3 walk left
uint8_t stateDelays[SPR_STATE_COUNT] = {5,5,5,5,20,20}; //delays between each frame for each state
uint8_t dashDelay = 8; //delay between dash frames


//this is an extended spriteDefine from the library, but it adds the initial frame we want to use
void mySpriteDefine(uint8_t s, uint32_t addr, uint8_t size, uint8_t clut, uint8_t layer, uint8_t frame, uint16_t x, uint16_t y, bool wantVisible, uint8_t typeOf)
{
	uint32_t offset = (uint32_t)SPR_SIZE_SQUARED + (uint32_t) frame;
	spriteDefine(s, addr + offset, size, clut, layer); //information for the vicky
	spriteDefine(s+10, addr + (uint32_t)(10)*(uint32_t)(SPR_SIZE_SQUARED), size, clut, layer); //dash information

	spriteStatuses[s].rightOrLeft = true;
	
	spriteSetPosition(s,x,y);
	spriteSetVisible(s, wantVisible);
	spriteSetVisible(s+10, wantVisible);
	spriteSetPosition(s,x-SPR_SIZE,y);
	
	spriteStatuses[s].addr = addr;
	spriteStatuses[s].frame = frame;
	
	spriteStatuses[s].isDashing = false;
	
	spriteStatuses[s].x = x;
	spriteStatuses[s].y = y;
	
	spriteStatuses[s].sx = 0;
	spriteStatuses[s].sy = 0;
	
	spriteStatuses[s].timer.units = TIMER_FRAMES;
	spriteStatuses[s].timer.cookie = s;
	
	switch(typeOf)
	{
		case 0: //character sprites
			spriteStatuses[s].state = 0;
			spriteStatuses[s].minIndexForState = (uint8_t *)malloc(sizeof(uint8_t) * SPR_STATE_COUNT);
			spriteStatuses[s].maxIndexForState = (uint8_t *)malloc(sizeof(uint8_t) * SPR_STATE_COUNT);
			spriteStatuses[s].minIndexForState[0]=0; //idle looking right
			spriteStatuses[s].maxIndexForState[0]=0;
			spriteStatuses[s].minIndexForState[1]=2; //idle looking left
			spriteStatuses[s].maxIndexForState[1]=2;
			spriteStatuses[s].minIndexForState[2]=0; //walk right
			spriteStatuses[s].maxIndexForState[2]=1;
			spriteStatuses[s].minIndexForState[3]=2; //walk left
			spriteStatuses[s].maxIndexForState[3]=3;
			spriteStatuses[s].minIndexForState[4]=1; //dash right
			spriteStatuses[s].maxIndexForState[4]=1;
			spriteStatuses[s].minIndexForState[5]=3; //dash left
			spriteStatuses[s].maxIndexForState[5]=3;
			break;
		case 1: //dash sprites
			spriteStatuses[s].state = 0;
			spriteStatuses[s].minIndexForState  = (uint8_t *)malloc(sizeof(uint8_t) * 2);
			spriteStatuses[s].maxIndexForState  = (uint8_t *)malloc(sizeof(uint8_t) * 2);
			spriteStatuses[s].minIndexForState[0]=0; //dash towards right, so ejecting left
			spriteStatuses[s].maxIndexForState[0]=3;
			spriteStatuses[s].minIndexForState[1]=4; //dash towards left, so ejecting right
			spriteStatuses[s].maxIndexForState[1]=7;
			break;
	}
}

//this is used to change the address of the graphics pointed to by the sprite. every sprite frame change should use this
void mySetSpriteAddr(uint8_t s, uint32_t address) {
	uint16_t sprite = VKY_SP0_CTRL + (s * 8); //start from sprite 0's address but offset to the right sprite
	POKEA(sprite + 1, address);
}

