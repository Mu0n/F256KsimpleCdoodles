#define F256LIB_IMPLEMENTATION

#define BITMAP_BASE 0x10000

#define TIMER_FRAMES 0
#define TIMER_COOKIE 0

#define LCD_CMD_CMD 0xDD40  //Write Command Here
#define LCD_RST 0x10        //0 to Reset (RSTn)
#define LCD_BL  0x20        //1 = ON, 0 = OFF 
#define LCD_WIN_X 0x2A    // window X command 
#define LCD_WIN_Y 0x2B    // window Y command
#define LCD_WRI   0x2C    //write command

//Read Only
#define LCD_TE  0x40        //Tear Enable 
#define LCD_CMD_DTA 0xDD41  //Write Data (For Command) Here
//Always Write in Pairs, otherwise the State Machine will Lock
#define LCD_PIX_LO   0xDD42 //{G[2:0], B[4:0]}
#define LCD_PIX_HI   0xDD43 //{R[4:0], G[5:3]}
#define LCD_CTRL_REG 0xDD44


#include "f256lib.h"

//STRUCTS
struct timer_t pauseTimer;

EMBED(mac, "../assets/mom.bin", 0x10000);

void clearVisible(uint8_t, uint8_t, uint8_t);
void gotoLCDXY(uint8_t, uint16_t);
void lilpause(uint8_t);
bool setTimer(const struct timer_t *);
uint8_t getTimerAbsolute(uint8_t);
void displayImage();

void gotoLCDXY(uint8_t, uint16_t)
{
	
}
//Sends a kernel based timer. You must prepare a timer_t struct first and initialize its fields
bool setTimer(const struct timer_t *timer)
{
    *(uint8_t*)0xf3 = timer->units;
    *(uint8_t*)0xf4 = timer->absolute;
    *(uint8_t*)0xf5 = timer->cookie;
    kernelCall(Clock.SetTimer);
	return !kernelError;
}
//getTimerAbsolute:
//This is essential if you want to retrigger a timer properly. The old value of the absolute
//field has a high chance of being desynchronized when you arrive at the moment when a timer
//is expired and you must act upon it.
//get the value returned by this, add the delay you want, and use setTimer to send it off
//ex: myTimer.absolute = getTimerAbsolute(TIMES_SECONDS) + TIMER_MYTIMER_DELAY
uint8_t getTimerAbsolute(uint8_t units)
{
    *(uint8_t*)0xf3 = units | 0x80;
    return kernelCall(Clock.SetTimer);
}

void lilpause(uint8_t timedelay)
{
	bool noteExitFlag = false;
	pauseTimer.absolute = getTimerAbsolute(TIMER_FRAMES) + timedelay;
	setTimer(&pauseTimer);
	noteExitFlag = false;
	while(!noteExitFlag)
	{
		kernelNextEvent();
		if(kernelEventData.type == kernelEvent(timer.EXPIRED))
		{
			switch(kernelEventData.timer.cookie)
			{
			case TIMER_COOKIE:
				noteExitFlag = true;
				break;
			}
		}
	}
}

void clearVisible(uint8_t red, uint8_t green, uint8_t blue)
{
	uint8_t i;
	uint16_t j;
	POKE(LCD_CMD_CMD, LCD_WIN_X);
	POKE(LCD_CMD_DTA, 0); //xstart high
	POKE(LCD_CMD_DTA, 0); //xstart low
	POKE(LCD_CMD_DTA, 0); //xend high
	POKE(LCD_CMD_DTA, 239); //xend low

	POKE(LCD_CMD_CMD, LCD_WIN_Y);
	POKE(LCD_CMD_DTA, 0); //xstart high
	POKE(LCD_CMD_DTA, 20); //xstart low
	POKE(LCD_CMD_DTA, 0x01); //xend high
	POKE(LCD_CMD_DTA, 0x40); //xend low
	
	POKE(LCD_CMD_CMD, LCD_WRI);
	
	for(j=0;j<280;j++)
	{
		for(i=0;i<240;i++)
		{
			POKE(LCD_PIX_LO,blue&0x0F);POKE(LCD_PIX_HI,red<<4 | (green&0x0F));
		}
	}
}

void displayImage()
{
	uint32_t i;
	uint32_t j;
	uint32_t index = 0;
	POKE(LCD_CMD_CMD, LCD_WIN_X);
	POKE(LCD_CMD_DTA, 0); //xstart high
	POKE(LCD_CMD_DTA, 0); //xstart low
	POKE(LCD_CMD_DTA, 0); //xend high
	POKE(LCD_CMD_DTA, 239); //xend low

	POKE(LCD_CMD_CMD, LCD_WIN_Y);
	POKE(LCD_CMD_DTA, 0); //xstart high
	POKE(LCD_CMD_DTA, 20); //xstart low
	POKE(LCD_CMD_DTA, 0x01); //xend high
	POKE(LCD_CMD_DTA, 0x40); //xend low
	
	POKE(LCD_CMD_CMD, LCD_WRI);
	
	for(j=0;j<280;j++)
	{
		for(i=0;i<240;i++)
		{
			POKE(LCD_PIX_LO, FAR_PEEK((uint32_t)BITMAP_BASE + index));
			POKE(LCD_PIX_HI, FAR_PEEK((uint32_t)BITMAP_BASE + index + 1));
			index+=2;
		}
	}

}

int main(int argc, char *argv[]) {
	
	clearVisible(0,0,0);
	displayImage();
	
	while(true);
	return 0;}
