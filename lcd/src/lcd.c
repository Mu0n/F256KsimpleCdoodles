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
#define LCD_RD    0x2E    //read  command

//Read Only
#define LCD_TE  0x40        //Tear Enable 
#define LCD_CMD_DTA 0xDD41  //Write Data (For Command) Here
//Always Write in Pairs, otherwise the State Machine will Lock
#define LCD_PIX_LO   0xDD42 //{G[2:0], B[4:0]}
#define LCD_PIX_HI   0xDD43 //{R[4:0], G[5:3]}
#define LCD_CTRL_REG 0xDD44

#define LCD_PURE_RED  0xF800
#define LCD_PURE_GRN  0x07E0
#define LCD_PURE_BLU  0x001F
#define LCD_PURE_WHI  0xFFFF
#define LCD_PURE_BLK  0x0000

#include "f256lib.h"
#include "../src/muUtils.h" //contains helper functions I often use


EMBED(mac, "../assets/bomb2.bin", 0x10000);

uint16_t ccycle[3] = {LCD_PURE_RED, LCD_PURE_GRN, LCD_PURE_BLU};
void clearVisible(uint16_t);
void gotoLCDXY(uint8_t, uint16_t);
void lilpause(uint8_t);
bool setTimer(const struct timer_t *);
uint8_t getTimerAbsolute(uint8_t);
void displayImage();

void gotoLCDXY(uint8_t, uint16_t)
{
	
}

void clearVisible(uint16_t colorWord)
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
			POKEW(LCD_PIX_LO,colorWord);
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
	uint16_t r1,r2,r3,r4,r5,r6;
	uint8_t i=0;
	
	POKE(0xD6A0, 0b01111111);
	
	displayImage();
	while(true);
	while(true)
	{
		r1 = randomRead();
		r2 = randomRead();
		r3 = randomRead();
		r4 = randomRead();
		r5 = randomRead();
		r6 = randomRead();
		//clearVisible(ccycle[i++]);
		if(i>2)i=0;
		POKE(0xD6A7, HIGH_BYTE(r1)); //power blue
		POKE(0xD6A8, LOW_BYTE(r1));  //power green
		POKE(0xD6A9, HIGH_BYTE(r2)); //power red
		POKE(0xD6AA, LOW_BYTE(r2));  //media blue
		POKE(0xD6AB, HIGH_BYTE(r3)); //media green
		POKE(0xD6AC, LOW_BYTE(r3));  //media red
		POKE(0xD6AD, HIGH_BYTE(r4)); //lock blue
		POKE(0xD6AE, LOW_BYTE(r4));  //lock green 
		POKE(0xD6AF, HIGH_BYTE(r5));  //lock red
		POKE(0xD6B3, LOW_BYTE(r5)); //lock blue
		POKE(0xD6B4, HIGH_BYTE(r6));  //lock green 
		POKE(0xD6B5, LOW_BYTE(r6));  //lock red
		if(i%2==0) POKE(0xD6A0, 0b01101111);
		else POKE(0xD6A0,0b01111111);
	}
	displayImage();
	
	while(true);
	return 0;}
