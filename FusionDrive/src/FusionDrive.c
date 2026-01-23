
//DEFINES
#define F256LIB_IMPLEMENTATION

#define NES_CTRL    0xD880

#define NES_CTRL_TRIG 0b10000000
#define NES_STAT_DONE 0b01000000
//D880 settings
//        7  6  5  4 |  3  2   1  0
// NES_TRIG XX XX XX | XX MODE XX NES_EN
#define NES_CTRL_MODE_NES  0b00000001
#define NES_CTRL_MODE_SNES 0b00000101

#define NES_STAT    0xD880
#define NES_PAD0    0xD884
#define NES_PAD1    0xD886
#define NES_A      0x7F
#define NES_B      0xBF
#define NES_SELECT 0xDF
#define NES_START  0xEF
#define NES_UP     0xF7
#define NES_DOWN   0xFB
#define NES_LEFT   0xFD
#define NES_RIGHT  0xFE

#define ABS(a) ((a) < 0 ? -(a) : (a)) 

#define TIMER_SECONDS 1
#define TIMER_CAR_COOKIE 0
#define TIMER_LANE_COOKIE 1
#define TIMER_CAR_FORCE_COOKIE 2
#define TIMER_CAR_DELAY 1
#define TIMER_LANE_DELAY 3
#define TIMER_CAR_FORCE_DELAY 3
#define TIMER_ROAD_TWIST_COOKIE 3
#define TIMER_ROAD_TWIST_DELAY 10

#define VKY_SP0_CTRL  0xD900 //Sprite #0’s control register
#define VKY_SP0_AD_L  0xD901 // Sprite #0’s pixel data address register
#define VKY_SP0_AD_M     0xD902
#define VKY_SP0_AD_H     0xD903
#define VKY_SP0_POS_X_L  0xD904 // Sprite #0’s X position register
#define VKY_SP0_POS_X_H  0xD905
#define VKY_SP0_POS_Y_L  0xD906 // Sprite #0’s Y position register
#define VKY_SP0_POS_Y_H  0xD907
#define SPR_CLUT_COLORS       32


#define T0_PEND     0xD660
#define T0_MASK     0xD66C

#define T0_CTR      0xD650 //master control register for timer0, write.b0=ticks b1=reset b2=set to last value of VAL b3=set count up, clear count down
#define T0_STAT     0xD650 //master control register for timer0, read bit0 set = reached target val

#define CTR_INTEN   0x80  //present only for timer1? or timer0 as well?
#define CTR_ENABLE  0x01
#define CTR_CLEAR   0x02
#define CTR_LOAD    0x04
#define CTR_UPDOWN  0x08

#define T0_VAL_L    0xD651 //current 24 bit value of the timer
#define T0_VAL_M    0xD652
#define T0_VAL_H    0xD653

#define T0_CMP_CTR  0xD654 //b0: t0 returns 0 on reaching target. b1: CMP = last value written to T0_VAL
#define T0_CMP_L    0xD655 //24 bit target value for comparison
#define T0_CMP_M    0xD656
#define T0_CMP_H    0xD657

#define T0_CMP_CTR_RECLEAR 0x01
#define T0_CMP_CTR_RELOAD  0x02

#define MIDI_BASE   0x38000 //gives a nice 128kb until the parsed version happens
#define MIDI_PARSED 0x58000 //end of ram is 0x7FFFF
//offsets from MIDI_PARSED
#define AME_DELTA 0
#define AME_BYTECOUNT 4
#define AME_MSG 5

#define MIDI_EVENT_FAR_SIZE 8


#define PAL_BASE    0x10000
#define BITMAP_BASE      0x20000
#define BITMAP_BASE2      0x44000
#define SPR_FRONT_L 0x10400
#define SPR_FRONT_R 0x10800
#define SPR_MIDDLE_L 0x10C00
#define SPR_MIDDLE_R 0x11000
#define SPR_BACK_L 0x011400
#define SPR_BACK_R 0x011800   
#define SPR_YEL_L 0x011C00    
#define SPR_YEL_R 0x012000    
#define SPR_YEL_FAR1 0x012400
#define SPR_YEL_FAR2 0x012800
#define SPR_YEL_FAR3 0x012C00             
#define TIMER_FRAMES 0

#define TOP_SPEED 4
#define TOP_MIN_X 42
#define TOP_MAX_X 278
#define TOP_FORCE 2
#define ENMY_ENTRY_Y 170

#define MUSIC_BASE 0x50000

//INCLUDES
#include "f256lib.h"
#include <stdlib.h>
#include "../src/muUtils.h" //contains helper functions I often use
#include "../src/muMidi.h"  //contains basic MIDI functions I often use
#include "../src/muMidiPlay2.h"  //digested .dim midi file player
#include "../src/muTimer0Int.h"  //contains timer0 routines

EMBED(palback, "../assets/Urban4.data.pal", 0x10000); //1kb
EMBED(carF, "../assets/carfront.bin",0x10400); //2kb
EMBED(carM, "../assets/carmid.bin",0x10C00);   //2kb
EMBED(carB, "../assets/carback.bin",0x11400); //2kb
EMBED(yellow, "../assets/yellow.bin",0x11C00); //5kb
//next would be at 0x13000
EMBED(backbmp, "../assets/Urban4.data", 0x20000);
EMBED(back2bmp, "../assets/Urban4.data", 0x44000);

//GLOBALS
struct timer_t carTimer, laneTimer, carForceTimer, roadTwistDelay;

bool readyForNextNote = 1; //interrupt-led flag to signal the loop it's ready for the next MIDI event
int16_t tempAddr; //used to compute addresses for pokes
uint16_t *parsers; //indices for the various type 1 tracks during playback
uint32_t *targetParse; //will pick the default 0x20000 if the file doesn't load
uint8_t nn=4, dd=2, cc=24, bb=8; //nn numerator of time signature, dd= denom. cc=nb of midi clocks in metro click. bb = nb of 32nd notes in a beat
uint8_t gSineIndex=0;
uint8_t z=0; //index for SIN lookup in road twist

int16_t SIN[] = {
       0,    1,    3,    4,    6,    7,    9,   10,
      12,   14,   15,   17,   18,   20,   21,   23,
      24,   25,   27,   28,   30,   31,   32,   34,
      35,   36,   38,   39,   40,   41,   42,   44,
      45,   46,   47,   48,   49,   50,   51,   52,
      53,   54,   54,   55,   56,   57,   57,   58,
      59,   59,   60,   60,   61,   61,   62,   62,
      62,   63,   63,   63,   63,   63,   63,   63,
      64,   63,   63,   63,   63,   63,   63,   63,
      62,   62,   62,   61,   61,   60,   60,   59,
      59,   58,   57,   57,   56,   55,   54,   54,
      53,   52,   51,   50,   49,   48,   47,   46,
      45,   44,   42,   41,   40,   39,   38,   36,
      35,   34,   32,   31,   30,   28,   27,   25,
      24,   23,   21,   20,   18,   17,   15,   14,
      12,   10,    9,    7,    6,    4,    3,    1,
       0,   -1,   -3,   -4,   -6,   -7,   -9,  -10,
     -12,  -14,  -15,  -17,  -18,  -20,  -21,  -23,
     -24,  -25,  -27,  -28,  -30,  -31,  -32,  -34,
     -35,  -36,  -38,  -39,  -40,  -41,  -42,  -44,
     -45,  -46,  -47,  -48,  -49,  -50,  -51,  -52,
     -53,  -54,  -54,  -55,  -56,  -57,  -57,  -58,
     -59,  -59,  -60,  -60,  -61,  -61,  -62,  -62,
     -62,  -63,  -63,  -63,  -63,  -63,  -63,  -63,
     -64,  -63,  -63,  -63,  -63,  -63,  -63,  -63,
     -62,  -62,  -62,  -61,  -61,  -60,  -60,  -59,
     -59,  -58,  -57,  -57,  -56,  -55,  -54,  -54,
     -53,  -52,  -51,  -50,  -49,  -48,  -47,  -46,
     -45,  -44,  -42,  -41,  -40,  -39,  -38,  -36,
     -35,  -34,  -32,  -31,  -30,  -28,  -27,  -25,
     -24,  -23,  -21,  -20,  -18,  -17,  -15,  -14,
     -12,  -10,   -9,   -7,   -6,   -4,   -3,   -1,
};

const uint16_t linesForCurves[]={0,440,450,460,470};
//GLOBALS
typedef struct sprStatus
{
	uint8_t s; //sprite index in Vicky
	int16_t x,y,z; //position
	bool rightOrLeft; //last facing
	bool isDashing;
	uint32_t addr; //base address
	uint8_t frame; //frame into view
	uint16_t sx, sy; //speed
	int8_t vx, vy; //velocity
	int8_t ax, ay; //acceleration
	struct timer_t timer; //animation timer;
	uint8_t cookie; //cookie for timer
	uint8_t state; //which state: 0 idle, 1 walk right, 2 walk left, etc
	uint8_t *minIndexForState; //minimum index for given state
	uint8_t *maxIndexForState; //maximum index for given state
} sprStatus;

struct sprStatus car_back_L, car_back_R, car_mid_L, car_mid_R, car_front_L, car_front_R,
                 car_yel_L, car_yel_R;


//FUNCTION PROTOTYPES
void setup(void);

void mySetCar(uint8_t, uint32_t, uint8_t, uint8_t, uint8_t, uint8_t, uint16_t, uint16_t, uint8_t, bool, struct sprStatus *);
void updateCarPos();
void setCarPos(int16_t, int16_t);
void swapColors(uint8_t, uint8_t);
void setEnemyPos(struct sprStatus*);

	
void setCarPos(int16_t x, int16_t y)
{
	int16_t midx, frontx;
	
	midx  = x-(x-160)/18;
	frontx= x-(x-160)/12;
	spriteSetPosition(0,x,y);
	spriteSetPosition(1,x+32,y);
	spriteSetPosition(2,midx,y);
	spriteSetPosition(3,midx+32,y);
	spriteSetPosition(4,frontx,y);
	spriteSetPosition(5,frontx+32,y);
}

void setEnemyPos(struct sprStatus *theSpr)
{
	uint32_t tempAddr=0;
	
	switch(theSpr->z)
	{
		case 3:
		    tempAddr = theSpr->addr + (uint32_t)4096;
			spriteDefine(theSpr->s, tempAddr , 32, 0, 1);
			spriteSetVisible(theSpr->s+1,false);
			break;
		case 2:
			tempAddr =  theSpr->addr + (uint32_t)3072;
			spriteDefine(theSpr->s, tempAddr, 32, 0, 1);
			spriteSetVisible(theSpr->s+1,false);
			break;
		case 1:
			tempAddr = theSpr->addr + (uint32_t)2048;
			spriteDefine(theSpr->s, tempAddr, 32, 0, 1);
			spriteSetVisible(theSpr->s+1,false);
			break;
		case 0:
			tempAddr =  theSpr->addr;
			spriteDefine(theSpr->s,	tempAddr , 32, 0, 0);
			spriteDefine(theSpr->s+1, tempAddr + (uint32_t)1024, 32, 0, 0);
			spriteSetVisible(theSpr->s+1,true);
			break;
	}
	spriteSetVisible(theSpr->s,true);
	textGotoXY(0,15);
	
	printf("ex:%d ey:%d ez:%d        ",theSpr->x, theSpr->y, theSpr->z);
	spriteSetPosition(theSpr->s,theSpr->x  - (theSpr->z==0?16:0),theSpr->y);
	spriteSetPosition(theSpr->s+1,theSpr->x+32 - (theSpr->z==0?16:0),theSpr->y);
	theSpr->y += theSpr->sy;
	if(theSpr->y>=220) 
	{
		spriteDefine(theSpr->s,tempAddr,32,0,0);
		spriteDefine(theSpr->s+1,tempAddr + (uint32_t)1024,32,0,0);
	spriteSetVisible(theSpr->s,true);
	spriteSetVisible(theSpr->s+1,true);
	}
	else
	{
		spriteDefine(theSpr->s,tempAddr,32,0,1);
	}
	if(theSpr->y>240) theSpr->y = ENMY_ENTRY_Y;
	theSpr->z = 3 - (theSpr->y - ENMY_ENTRY_Y)/12;
	if(theSpr->z < 0) theSpr->z = 0;
	
}
void updateCarPos()
{

	kernelNextEvent();
	if(kernelEventData.type == kernelEvent(timer.EXPIRED))
	{		
		switch(kernelEventData.timer.cookie)
		{
			case TIMER_CAR_COOKIE:
			
				
			
				carTimer.absolute = getTimerAbsolute(TIMER_FRAMES) + TIMER_CAR_DELAY;
				setTimer(&carTimer);
			
				POKE(NES_CTRL, NES_CTRL_MODE_NES | NES_CTRL_TRIG);
				asm("NOP");
				asm("NOP");
				POKE(NES_CTRL, NES_CTRL_MODE_NES);
				while((PEEK(NES_CTRL) & NES_STAT_DONE) != NES_STAT_DONE)
					;
				if(PEEK(NES_PAD1)==NES_LEFT)
				{
					car_yel_L.x --;
				}
				else if(PEEK(NES_PAD1) == NES_RIGHT)
				{
					car_yel_L.x ++;
				}
				if(PEEK(NES_PAD1)==NES_DOWN)
				{
					car_yel_L.z--;
				}
				else if(PEEK(NES_PAD1)==NES_UP)
				{
					car_yel_L.z++;
				}
				if(PEEK(NES_PAD0)==NES_LEFT)
				{
					car_back_L.ax = -TOP_FORCE;
				}
				else if(PEEK(NES_PAD0) == NES_RIGHT)
				{
					car_back_L.ax = TOP_FORCE;
				}
				else 
				{
					car_back_L.ax = 0;
				}
				car_back_L.x += car_back_L.vx;
				if(car_back_L.x < TOP_MIN_X) 
				{
					car_back_L.vx = 0;
					car_back_L.x = TOP_MIN_X;
				}
				if(car_back_L.x > TOP_MAX_X) 
				{
					car_back_L.vx = 0;
					car_back_L.x = TOP_MAX_X;
				}

				setCarPos(car_back_L.x, car_back_L.y);
				textGotoXY(0,10);
				printf("x:%d vx:%d ax:%d              ", car_back_L.x, car_back_L.vx, car_back_L.ax);
				break;
			case TIMER_LANE_COOKIE:
				swapColors(112,146);
				laneTimer.absolute = getTimerAbsolute(TIMER_FRAMES) + TIMER_LANE_DELAY;
				setTimer(&laneTimer);
				break;
			case TIMER_CAR_FORCE_COOKIE:
				setEnemyPos(&car_yel_L);
				if(car_back_L.ax == 0) //no longer pushing to the sides; start the deceleration process due to friction
				{
					if(car_back_L.vx < 0 && car_back_L.vx != 0) car_back_L.vx++;
					if(car_back_L.vx > 0 && car_back_L.vx != 0) car_back_L.vx--;
				}
				else car_back_L.vx += car_back_L.ax; //top speed limitation, for both negative and positive speed orientation
				
				if(car_back_L.vx > TOP_SPEED) car_back_L.vx = TOP_SPEED; //speed limiters
				if(car_back_L.vx < -TOP_SPEED) car_back_L.vx = -TOP_SPEED;
				//if(car_back_L.vx == 0) car_back_L.ax = 0;
				
				carForceTimer.absolute = getTimerAbsolute(TIMER_FRAMES)+TIMER_CAR_FORCE_DELAY;
				setTimer(&carForceTimer);
				break;
			case TIMER_ROAD_TWIST_COOKIE:
			/*
				roadTwistDelay.absolute = getTimerAbsolute(TIMER_FRAMES) + TIMER_ROAD_TWIST_DELAY;
				setTimer(&roadTwistDelay);
				*/
				break;
				
		}
	}
}

void swapColors(uint8_t c1, uint8_t c2)
{
	uint8_t tempR=0, tempG=0, tempB=0;
	
	POKE(MMU_IO_CTRL,1);
	tempR = PEEK(VKY_GR_CLUT_0+4*c2);
	tempG = PEEK(VKY_GR_CLUT_0+4*c2+1);
	tempB = PEEK(VKY_GR_CLUT_0+4*c2+2);
	POKE(VKY_GR_CLUT_0+4*c2,    PEEK(VKY_GR_CLUT_0+4*c1)); 
	POKE(VKY_GR_CLUT_0+4*c2+1,	PEEK(VKY_GR_CLUT_0+4*c1+1));
	POKE(VKY_GR_CLUT_0+4*c2+2,	PEEK(VKY_GR_CLUT_0+4*c1+2));
	
	POKE(VKY_GR_CLUT_0+4*c1,    tempR); 
	POKE(VKY_GR_CLUT_0+4*c1+1,	tempG);
	POKE(VKY_GR_CLUT_0+4*c1+2,	tempB);
	
	POKE(MMU_IO_CTRL,0);
}
void setup()
{
	uint32_t c;
	
	POKE(MMU_IO_CTRL, 0x00);
	// XXX GAMMA  SPRITE   TILE  | BITMAP  GRAPH  OVRLY  TEXT
	POKE(VKY_MSTR_CTRL_0, 0b00101111); //sprite,graph,overlay,text
	// XXX XXX  FON_SET FON_OVLY | MON_SLP DBL_Y  DBL_X  CLK_70
	POKE(VKY_MSTR_CTRL_1, 0b00010100); //font overlay, double height text, 320x240 at 60 Hz;
	POKE(VKY_LAYER_CTRL_0, 0b00010000); //bitmap 0 in layer 0, bitmap 1 in layer 1
	POKE(VKY_LAYER_CTRL_1, 0b00000010); //bitmap 2 in layer 2
	POKE(0xD00D,0x00); //force black graphics background
	POKE(0xD00E,0x00);
	POKE(0xD00F,0x00);
	POKE(MMU_IO_CTRL,1);  //MMU I/O to page 1
	// Set up CLUT0.
	for(c=0;c<1023;c++) 
	{
		POKE(VKY_GR_CLUT_0+c, FAR_PEEK(PAL_BASE+c));
	}

	
	POKE(MMU_IO_CTRL,0);
	bitmapSetActive(2);
	bitmapSetAddress(2,BITMAP_BASE);
	bitmapSetCLUT(0);
	/*
	bitmapSetActive(1);
	bitmapSetCLUT(0);
	bitmapSetAddress(1,BITMAP_BASE2);
	*/
	/*
	for(uint16_t x=0;x<320;x++)
	{
		for(uint16_t y=0;y<140;y++)
		{
			FAR_POKE(BITMAP_BASE2 + y*320+x,0);
		}
	}
	*/
	bitmapSetVisible(0,false);
	bitmapSetVisible(1,false);
	bitmapSetVisible(2,true); //furthermost to act as a real background

	//bitmapSetActive(2);
	
	printf("yo");
	hitspace();
/*
void mySetCar(uint8_t s, uint32_t addr, uint8_t size, uint8_t clut, uint8_t layer, uint8_t frame, uint16_t x, uint16_t y, uint8_t z, bool wantVisible, struct sprStatus *theCarPart) 
*/

	mySetCar(0, SPR_BACK_L, 32, 0, 1, 0, 128, 220, 0, true, &car_back_L);
	mySetCar(1, SPR_BACK_R, 32, 0, 1, 0, 160, 220, 0, true, &car_back_R);
	mySetCar(2, SPR_MIDDLE_L, 32, 0, 1, 0, 128, 220, 0, true, &car_mid_L);
	mySetCar(3, SPR_MIDDLE_R, 32, 0, 1, 0, 160, 220, 0, true, &car_mid_R);
	mySetCar(4, SPR_FRONT_L, 32, 0, 1, 0, 128, 220, 0, true, &car_front_L);
	mySetCar(5, SPR_FRONT_R, 32, 0, 1, 0, 160, 220, 0, true, &car_front_R);
	
	mySetCar(6, SPR_YEL_L, 32, 0, 1, 0, 165, ENMY_ENTRY_Y, 3, false, &car_yel_L);
	//sprite 7 is reserved for this yellow car
	car_yel_L.sy=1;
	
	setCarPos(128,220);

	//set NES_CTRL
	POKE(NES_CTRL,NES_CTRL_MODE_NES);

	POKE(MMU_IO_CTRL, 0x00);
	
}

void mySetCar(uint8_t s, uint32_t addr, uint8_t size, uint8_t clut, uint8_t layer, uint8_t frame, uint16_t x, uint16_t y, uint8_t z, bool wantVisible, struct sprStatus *theCarPart) 
{
	spriteDefine(s, addr, size, clut, layer);
	spriteSetPosition(s,x,y);
	spriteSetVisible(s,wantVisible);

	theCarPart->s = s;
	theCarPart->x = x;
	theCarPart->y = y;
	theCarPart->z = z;
	theCarPart->addr = addr;
	theCarPart->frame = frame;
	theCarPart->sx = 0;
	theCarPart->sy = 0;
}

			
void prepTimers()
	{
	carTimer.units = TIMER_FRAMES;
	carTimer.absolute = getTimerAbsolute(TIMER_FRAMES) + TIMER_CAR_DELAY;
	carTimer.cookie = TIMER_CAR_COOKIE;
	setTimer(&carTimer);

	laneTimer.units = TIMER_FRAMES;
	laneTimer.absolute = getTimerAbsolute(TIMER_FRAMES) + TIMER_LANE_DELAY;
	laneTimer.cookie = TIMER_LANE_COOKIE;
	setTimer(&laneTimer);	
	
	carForceTimer.units = TIMER_FRAMES;
	carForceTimer.absolute = getTimerAbsolute(TIMER_FRAMES) + TIMER_CAR_FORCE_DELAY;
	carForceTimer.cookie = TIMER_CAR_FORCE_COOKIE;
	setTimer(&carForceTimer);
	
	roadTwistDelay.units = TIMER_FRAMES;
	roadTwistDelay.absolute = getTimerAbsolute(TIMER_FRAMES) + TIMER_ROAD_TWIST_DELAY;
	roadTwistDelay.cookie = TIMER_ROAD_TWIST_COOKIE;
	setTimer(&roadTwistDelay);
	
	}
	
			
int main(int argc, char *argv[]) {
	bool exitFlag=false;
	uint16_t nextLine = 100;
	
	uint8_t index=0;
	bool isDone = false;
	
	setup();
	midiShutUp(false);
	
	resetInstruments(false); //resets all channels to piano, for sam2695
	midiShutUp(false); //ends trailing previous notes if any, for sam2695
	
	if(loadSMFile("media/midi/continen.mid",MUSIC_BASE) == 1) printf("error loading");
	
    initTrack(MUSIC_BASE);	
	for(uint16_t i=0;i<theOne.nbTracks;i++)	exhaustZeroes(i);
	resetTimer0();	
	setSOL();
	setSOL_line(0);
	//playEmbeddedDim(MUSIC_BASE);
	midiShutAllChannels(false);	
	uint8_t cycling =0;
	prepTimers();
	while(!exitFlag)
		{
		while(!isDone)
			{
				if(PEEK(INT_PENDING_0)&INT_TIMER_0) //when the timer0 delay is up, go here
					{
					POKE(INT_PENDING_0,INT_TIMER_0); //clear the timer0 delay
					playMidi(); //play the next chunk of the midi file, might deal with multiple 0 delay stuff
					}
					
				if(theOne.isWaiting == false) 
					{
					sniffNextMIDI(); //find next event to play, will cue up a timer0 delay
					}
				updateCarPos();
				
			//asm("sei");
			/*
				if(PEEK(INT_PENDING_0)&INT_VKY_SOL)
					{
					POKE(INT_PENDING_0,INT_VKY_SOL);	
*/			
			if(cycling++ == 0xFF) bitmapSetAddress(2,BITMAP_BASE+(int32_t)(SIN[index++]));
//					setSOL_line(480-150+(uint16_t)index);
//					if(index >150) index =0;
					//}
			
			//asm("cli");		
			}
			
			/*
			index++;
			if(index>4) index=0;
			POKEA(VKY_SP0_AD_L, frame[index]);
			*/
			
			
		}
	return 0;}
