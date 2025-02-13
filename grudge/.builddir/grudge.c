#include "D:\F256\llvm-mos\code\grudge\.builddir\trampoline.h"

//DEFINES
#define F256LIB_IMPLEMENTATION

#define TIMER_FRAMES 0
#define TIMER_SECONDS 1
#define TIMER_SPRANIM_COOKIE 0
#define TIMER_DSHANIM_COOKIE 10
#define TIMER_PAD_COOKIE 99

#define TIMER_LILPAUSE_COOKIE 100
#define TIMER_PAD_DELAY 2


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



#define NES_CTRL    0xD880

#define NES_CTRL_TRIG  0b10000000
#define NES_STAT_DONE  0b01000000
//D880 settings
//        7  6  5  4 |  3  2   1  0
// NES_TRIG XX XX XX | XX MODE XX NES_EN
#define NES_CTRL_MODE_NES  0b00000001
#define NES_CTRL_MODE_SNES 0b00000101

#define NES_STAT    0xD880
#define NES_PAD0    0xD884
#define NES_PAD1    0xD886
#define NES_PAD2    0xD888
#define NES_PAD3    0xD88A
#define NES_PAD_A      7
#define NES_PAD_B      6
#define NES_PAD_SELECT 5
#define NES_PAD_START  4
#define NES_PAD_UP     3
#define NES_PAD_DOWN   2
#define NES_PAD_LEFT   1
#define NES_PAD_RIGHT  0

#define SNES_PAD_B      7

#define SPEED_BASE 1
#define DASH_DISPLACE 24

#define PAL_BASE         0x10000
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

//INCLUDES
#include "f256lib.h"
#include "../src/muUtils.h" //contains helper functions I often use
#include <stdlib.h>

EMBED(palgrudge, "../assets/grudge.pal", 0x10000);//1kb
EMBED(boy1, "../assets/boy1.bin",   0x10400);//1kb
EMBED(thing, "../assets/thing.bin", 0x10800);//1kb
EMBED(girl1, "../assets/girl1.bin", 0x10C00);//1kb
EMBED(cath, "../assets/cath.bin",   0x11000);//1kb
EMBED(cat, "../assets/cat.bin",     0x11400);//1kb

EMBED(dash, "../assets/dash.bin",   0x13000); //2kb



//FUNCTION PROTOTYPES
void setup(void);
void updateFrames(void);
void mySpriteDefine(uint8_t, uint32_t, uint8_t, uint8_t, uint8_t, uint8_t, uint16_t, uint16_t, bool, uint8_t);
void checkNESPad(uint8_t, bool);
void lilpause(uint8_t);

//GLOBALS
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


sprStatus spriteStatuses[SPR_COUNT*2]; //hold characters sprite information

//states: 0=stand towards right, 1=stand towards left, 2 walk right, 3 walk left
uint8_t stateDelays[SPR_STATE_COUNT] = {5,5,5,5,20,20}; //delays between each frame for each state
uint8_t dashDelay = 8; //delay between dash frames

struct timer_t padTimer, lilPauseTimer; //delay for pad timer updating


void lilpause(uint8_t timedelay)
{
	bool noteExitFlag = false;
	lilPauseTimer.cookie = TIMER_LILPAUSE_COOKIE;
	lilPauseTimer.absolute = getTimerAbsolute(TIMER_FRAMES) + timedelay;
	setTimer(&lilPauseTimer);
	noteExitFlag = false;
	while(!noteExitFlag)
	{
		kernelNextEvent();
		if(kernelEventData.type == kernelEvent(timer.EXPIRED))
		{
			switch(kernelEventData.timer.cookie)
			{
			case TIMER_LILPAUSE_COOKIE:
				noteExitFlag = true;
				break;
			}
		}
	}
}

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
void mySetSpriteAddr(byte s, uint32_t address) {
	uint16_t sprite = VKY_SP0_CTRL + (s * 8); //start from sprite 0's address but offset to the right sprite
	POKEA(sprite + 1, address);
}


//updates all shown frame graphics to the current state of all sprites
void updateFrames()
{
	uint8_t i = 0;
	uint32_t offset;
	for(i=0;i<5;i++)
	{
		offset = (uint32_t)SPR_SIZE_SQUARED * (uint32_t) spriteStatuses[i].frame;
		mySetSpriteAddr(i,spriteStatuses[i].addr + offset);
		spriteStatuses[i].x +=  spriteStatuses[i].sx;
		spriteStatuses[i].y +=  spriteStatuses[i].sy;
		spriteSetPosition(i,spriteStatuses[i].x, spriteStatuses[i].y);
	}
	for(i=10;i<15;i++)
	{
		offset = (uint32_t)SPR_SIZE_SQUARED * (uint32_t) spriteStatuses[i].frame;
		mySetSpriteAddr(i,spriteStatuses[i].addr + offset);
		spriteSetPosition(i,spriteStatuses[i].x, spriteStatuses[i].y);
	}
}
	
	
	
	
void setup()
{
	uint32_t c;
	
	POKE(MMU_IO_CTRL, 0x00);
	// XXX GAMMA  SPRITE   TILE  | BITMAP  GRAPH  OVRLY  TEXT
	POKE(VKY_MSTR_CTRL_0, 0b00101111); //sprite,graph,overlay,text
	// XXX XXX  FON_SET FON_OVLY | MON_SLP DBL_Y  DBL_X  CLK_70
	POKE(VKY_MSTR_CTRL_1, 0b00000100); //font overlay, double height text, 320x240 at 60 Hz;
	POKE(VKY_LAYER_CTRL_0, 0b00000001); //bitmap 1 in layer 0, bitmap 0 in layer 1
	POKE(VKY_LAYER_CTRL_1, 0b00000010); //bitmap 2 in layer 2
	POKE(0xD00D,0xBB); //force dark gray graphics background
	POKE(0xD00E,0xBB);
	POKE(0xD00F,0xBB);
	POKE(MMU_IO_CTRL,1);  //MMU I/O to page 1
	// Set up CLUT0.
	for(c=0;c<1023;c++)
	{
		POKE(VKY_GR_CLUT_0+c, FAR_PEEK(PAL_BASE+c));
	}

	
	POKE(MMU_IO_CTRL,0);
	
	//set NES_CTRL
	POKE(NES_CTRL,NES_CTRL_MODE_NES);
	
	
	bitmapSetVisible(0,false);
	bitmapSetVisible(1,false);
	bitmapSetVisible(2,false);
	
	mySpriteDefine(0, SPR_BOY1_BASE, SPR_SIZE, 0, 0, 0, 100, 50, true, 0);
	mySpriteDefine(1, SPR_THNG_BASE, SPR_SIZE, 0, 0, 2, 200, 50, true, 0);
	mySpriteDefine(2, SPR_GLR1_BASE, SPR_SIZE, 0, 0, 0, 100, 80, true, 0);
	mySpriteDefine(3, SPR_CATH_BASE, SPR_SIZE, 0, 0, 2, 200, 80, true, 0);
	mySpriteDefine(4, SPR_CAT_BASE,  SPR_SIZE, 0, 0, 0, 100, 110, true, 0);
	
	mySpriteDefine(10, SPR_DASH_BASE, SPR_SIZE, 0, 0, 0, 0, 0, false, 1);
	mySpriteDefine(11, SPR_DASH_BASE, SPR_SIZE, 0, 0, 0, 0, 0, false, 1);
	mySpriteDefine(12, SPR_DASH_BASE, SPR_SIZE, 0, 0, 0, 0, 0, false, 1);
	mySpriteDefine(13, SPR_DASH_BASE, SPR_SIZE, 0, 0, 0, 0, 0, false, 1);
	mySpriteDefine(14, SPR_DASH_BASE, SPR_SIZE, 0, 0, 0, 0, 0, false, 1);
	
	spriteStatuses[0].state = 0;
	spriteStatuses[0].rightOrLeft = true;
	spriteStatuses[1].state = 1;
	spriteStatuses[1].rightOrLeft = false;
	spriteStatuses[2].state = 0;
	
	spriteStatuses[2].rightOrLeft = true;
	spriteStatuses[3].state = 1;
	spriteStatuses[3].rightOrLeft = false;
	spriteStatuses[4].state = 0;
	spriteStatuses[4].rightOrLeft = true;
	
	
	updateFrames();
	
	
	padTimer.units = TIMER_FRAMES;
	padTimer.cookie = TIMER_PAD_COOKIE;

}

void checkNESPad(uint8_t whichPad, bool nesOrSnes)
{
	uint16_t addr;
	bool rightOrLeft, bothAxis=false;
	uint8_t rightBButton;
	
	if(nesOrSnes==true) addr = NES_PAD0 + (uint16_t)whichPad*(uint16_t)2;
	else addr = NES_PAD0 + (uint16_t)(whichPad-4) * (uint16_t)2;
	
	if(!CHECK_BIT(PEEK(addr),NES_PAD_LEFT))
	{
		spriteStatuses[whichPad].sx=-SPEED_BASE;
		spriteStatuses[whichPad].state = SPR_STATE_WALK_L;
		spriteStatuses[whichPad].rightOrLeft = false;
	}
	else if(!CHECK_BIT(PEEK(addr),NES_PAD_RIGHT))
	{
		spriteStatuses[whichPad].sx=SPEED_BASE;
		spriteStatuses[whichPad].state = SPR_STATE_WALK_R;
		spriteStatuses[whichPad].rightOrLeft = true;
	}
	else
	{
		spriteStatuses[whichPad].state = spriteStatuses[whichPad].rightOrLeft?SPR_STATE_IDLE_R:SPR_STATE_IDLE_L;
		spriteStatuses[whichPad].sx=0;
	}
	if(!CHECK_BIT(PEEK(addr),NES_PAD_UP))
	{
		spriteStatuses[whichPad].sy=-SPEED_BASE;
		spriteStatuses[whichPad].state = spriteStatuses[whichPad].rightOrLeft?SPR_STATE_WALK_R:SPR_STATE_WALK_L;
	}
	else if(!CHECK_BIT(PEEK(addr),NES_PAD_DOWN))
	{
		spriteStatuses[whichPad].sy=SPEED_BASE;
		spriteStatuses[whichPad].state = spriteStatuses[whichPad].rightOrLeft?SPR_STATE_WALK_R:SPR_STATE_WALK_L;
	}
	else
	{
		spriteStatuses[whichPad].sy=0;
	}
	if(spriteStatuses[whichPad].sx == 0 && spriteStatuses[whichPad].sy == 0)
		spriteStatuses[whichPad].state = spriteStatuses[whichPad].rightOrLeft?SPR_STATE_IDLE_R:SPR_STATE_IDLE_L;
	
	rightBButton = nesOrSnes? NES_PAD_B: SNES_PAD_B;
	if(!CHECK_BIT(PEEK(addr),rightBButton))
	{
		if(spriteStatuses[whichPad].sx != 0 && spriteStatuses[whichPad].sy !=0) bothAxis = true;
		if(spriteStatuses[whichPad].isDashing || (spriteStatuses[whichPad].sx==0&& spriteStatuses[whichPad].sy==0)) return; //dashing forfeits control
	    spriteStatuses[whichPad].isDashing = true;
		rightOrLeft = spriteStatuses[whichPad].rightOrLeft;
	
		//set the character in a dashing state
		spriteStatuses[whichPad].state = rightOrLeft?4:5;
	
		//set the action dust cloud
		spriteStatuses[whichPad+10].x = spriteStatuses[whichPad].x;
		spriteStatuses[whichPad+10].y = (uint16_t)spriteStatuses[whichPad].y+(uint16_t)1;
		spriteStatuses[whichPad+10].state = rightOrLeft? 0:1;
		spriteStatuses[whichPad+10].frame = rightOrLeft? 0:4;
		//spriteSetPosition(whichPad+10,spriteStatuses[whichPad+10].x, spriteStatuses[whichPad+10].y);
	
		spriteSetPosition(whichPad+10,spriteStatuses[whichPad+10].x,spriteStatuses[whichPad+10].y);
		spriteSetVisible(whichPad+10,true);
		spriteStatuses[whichPad+10].timer.absolute = getTimerAbsolute(TIMER_FRAMES) + dashDelay;
		setTimer(&(spriteStatuses[whichPad+10].timer));
	
		spriteStatuses[whichPad].x += (bothAxis?17:DASH_DISPLACE) * spriteStatuses[whichPad].sx;
		spriteStatuses[whichPad].y += (bothAxis?17:DASH_DISPLACE) * spriteStatuses[whichPad].sy;
		spriteStatuses[whichPad].sx = 0;
		spriteStatuses[whichPad].sy = 0;
		spriteStatuses[whichPad].frame = rightOrLeft? 1:3;
		spriteStatuses[whichPad].state = rightOrLeft? 4:5;
	
		textGotoXY(0,0);textSetColor(4,0);
		printf("pad=%d x=%d y=%d state=%d frame=%d\n",whichPad,spriteStatuses[whichPad+10].x,spriteStatuses[whichPad+10].y, spriteStatuses[whichPad+10].state,spriteStatuses[whichPad+10].frame);
	}
}
int main(int argc, char *argv[]) {
	bool isDone = false;
	uint8_t state = 0, i=0;
	setup();

	//kick off a walking state
	spriteStatuses[0].state = 0; //idle right
	spriteStatuses[1].state = 2; //idle left
	spriteStatuses[2].state = 0; //idle right
	spriteStatuses[3].state = 2; //idle left
	spriteStatuses[4].state = 0; //idle right
	
	spriteStatuses[10].state = 0; //ejecting to left
	
	spriteStatuses[0].timer.absolute = getTimerAbsolute(TIMER_FRAMES) + stateDelays[spriteStatuses[0].state];
	setTimer(&(spriteStatuses[0].timer));
	spriteStatuses[1].timer.absolute = getTimerAbsolute(TIMER_FRAMES) + stateDelays[spriteStatuses[1].state];
	setTimer(&(spriteStatuses[1].timer));
	spriteStatuses[2].timer.absolute = getTimerAbsolute(TIMER_FRAMES) + stateDelays[spriteStatuses[2].state];
	setTimer(&(spriteStatuses[2].timer));
	spriteStatuses[3].timer.absolute = getTimerAbsolute(TIMER_FRAMES) + stateDelays[spriteStatuses[3].state];
	setTimer(&(spriteStatuses[3].timer));
	spriteStatuses[4].timer.absolute = getTimerAbsolute(TIMER_FRAMES) + stateDelays[spriteStatuses[4].state];
	setTimer(&(spriteStatuses[4].timer));
	
	
	padTimer.absolute = getTimerAbsolute(TIMER_FRAMES) + TIMER_PAD_DELAY;
	setTimer(&padTimer);
	
	while(!isDone)
		{
		POKE(NES_CTRL, NES_CTRL_MODE_NES | NES_CTRL_TRIG); //perform the trig
		kernelNextEvent();
		if(kernelEventData.type == kernelEvent(timer.EXPIRED))
			{
				switch(kernelEventData.timer.cookie)
				{
					case TIMER_PAD_COOKIE:
						while((PEEK(NES_STAT) & NES_STAT_DONE) != NES_STAT_DONE)//wait until the trig is done
									;
						for(i=0;i<4;i++)
							{
								checkNESPad(i, true);
							}
	
						//perform the trig
						POKE(NES_CTRL, NES_CTRL_MODE_SNES | NES_CTRL_TRIG);
						while((PEEK(NES_STAT) & NES_STAT_DONE) != NES_STAT_DONE)//wait until the trig is done
									;
						for(i=4;i<8;i++)
						{
								checkNESPad(i, false);
						}
	
						//SNES TODO
						//relaunch pad timer
						padTimer.absolute = getTimerAbsolute(TIMER_FRAMES) + TIMER_PAD_DELAY;
						setTimer(&padTimer);
						break;
					case 0 ... 9: //player animations to advance
						state = spriteStatuses[kernelEventData.timer.cookie].state;  //checks the state that it is in
						if(state == 4 || state == 5)
						{
							state = spriteStatuses[kernelEventData.timer.cookie].rightOrLeft ? 0:1;
							spriteStatuses[kernelEventData.timer.cookie].state = state;
							spriteStatuses[kernelEventData.timer.cookie].frame = state? 0:2;
						}
						spriteStatuses[kernelEventData.timer.cookie].timer.absolute = getTimerAbsolute(TIMER_FRAMES) +
																		stateDelays[spriteStatuses[kernelEventData.timer.cookie].state];
						setTimer(&(spriteStatuses[kernelEventData.timer.cookie].timer));
	
						//go to the next frame of the state
						spriteStatuses[kernelEventData.timer.cookie].frame++;
						//if we go over the last frame index of the state, go back to the beginning
						if(spriteStatuses[kernelEventData.timer.cookie].frame > spriteStatuses[kernelEventData.timer.cookie].maxIndexForState[state])
							spriteStatuses[kernelEventData.timer.cookie].frame = spriteStatuses[kernelEventData.timer.cookie].minIndexForState[state];
	
						updateFrames();
						break;
					case 10 ... 19: //dash animations to advance
						state = spriteStatuses[kernelEventData.timer.cookie].state;  //checks the state that it is in, either 0 or 1

						//go to the next frame of the state
						spriteStatuses[kernelEventData.timer.cookie].frame++;
						if(spriteStatuses[kernelEventData.timer.cookie].frame > spriteStatuses[kernelEventData.timer.cookie].maxIndexForState[state])
							{
								spriteStatuses[kernelEventData.timer.cookie].frame--;
							spriteSetVisible(kernelEventData.timer.cookie, false);
							spriteStatuses[kernelEventData.timer.cookie-10].isDashing = false;
							break;
							}
						else
						{
							spriteStatuses[kernelEventData.timer.cookie].timer.absolute = getTimerAbsolute(TIMER_FRAMES) + dashDelay;
							setTimer(&(spriteStatuses[kernelEventData.timer.cookie].timer));
						}
						break;
				}
				POKE(NES_CTRL, NES_CTRL_MODE_NES);
			}
		}
	return 0;}
