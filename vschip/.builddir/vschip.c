#include "D:\F256\llvm-mos\code\vschip\.builddir\trampoline.h"

#define F256LIB_IMPLEMENTATION

#define VS1053_0    	0xD700 //Register[0] for setting modes
#define VS1053_1 	    0xD701 //Register[1] for setting modes
#define VS1053_2   		0xD702 //Commands[0] for issuing commands
#define VS1053_3   		0xD703 //Commands[1] for issuing commands
#define VS1053_4   		0xD704 //Fifo count
#define VS1053_5  		0xD705 // Data?
#define VS1053_6		0xD706 // Data?
#define VS1053_7		0xD707 // Data ?

#define TIMER_FRAMES 0
#define TIMER_SECONDS 1

#define TIMER_TEXT_COOKIE 0
#define TIMER_TEXT_DELAY 1


#include "f256lib.h"

struct timer_t textTimer; //timer_t structure for setting timer through the kernel

bool setTimer(const struct timer_t *timer)
{
    *(uint8_t*)0xf3 = timer->units;
    *(uint8_t*)0xf4 = timer->absolute;
    *(uint8_t*)0xf5 = timer->cookie;
    kernelCall(Clock.SetTimer);
	return !kernelError;
}

uint8_t getTimerAbsolute(uint8_t units)
{
    *(uint8_t*)0xf3 = units | 0x80;
    return kernelCall(Clock.SetTimer);
}

void setup()
{
	textClear();
	textDefineForegroundColor(0,0xff,0xff,0xff);
    textGotoXY(0,0); textPrint("Displaying the VS1053b registers");

	
	textGotoXY(0,2); textPrint("0xD700: ");
	textGotoXY(0,3); textPrint("0xD701: ");
	textGotoXY(0,4); textPrint("0xD702: ");
	textGotoXY(0,5); textPrint("0xD703: ");
	textGotoXY(0,6); textPrint("0xD704: ");
	textGotoXY(0,7); textPrint("0xD705: ");
	textGotoXY(0,8); textPrint("0xD706: ");
	textGotoXY(0,9); textPrint("0xD707: ");
	
	
	textTimer.units = TIMER_FRAMES;
	textTimer.absolute = TIMER_TEXT_DELAY;
	textTimer.cookie = TIMER_TEXT_COOKIE;
	
	setTimer(&textTimer);
	
}

void refreshPrints()
{
	textGotoXY(10,2); printf("%02x  ",PEEK(0xD700));
	textGotoXY(10,3); printf("%02x  ",PEEK(0xD701));
	textGotoXY(10,4); printf("%02x  ",PEEK(0xD702));
	textGotoXY(10,5); printf("%02x  ",PEEK(0xD703));
	textGotoXY(10,6); printf("%02x  ",PEEK(0xD704));
	textGotoXY(10,7); printf("%02x  ",PEEK(0xD705));
	textGotoXY(10,8); printf("%02x  ",PEEK(0xD706));
	textGotoXY(10,9); printf("%02x  ",PEEK(0xD707));

}

void StefExample(){
	uint8_t i;
	uint16_t x,y;
	
	FILE *theMP3File;

/*
	theMP3File = fopen("one.mp3","r"); // open file in read mode

	if(theMP3File != NULL) {
		textPrint("Was able to open the file.\n");
		fclose(theMP3File);
	}
	*/
	
	POKE(VS1053_2, 0x42); //set stream mode for mp3 playing
	POKE(VS1053_3, 0x48); //set proper R/W directions for data
	POKE(VS1053_0, 0x21); //start the transaction
	POKE(VS1053_0, 0x00); //return to Zero
	
	// POKE(VS1053_2, 0x00);
	// POKE(VS1053_3, 0x00);
	// POKE(VS1053_0, 0x23); //read command register
	// PEEK(VS1053_2); //read cmd register 0, todo
	// PEEK(VS1053_3); //same reg 1
	for(i=0; i<0x80; i++)
	{
		asm("NOP"); //delay
	}
	
	POKE(0x0000, 0x80); //enable mmu edit, keep MMU 0 in playing
	POKE(0x000D, 0x08); // bring first 8K to page 0
	
	//setaxl = ?
	
	//fread(chunk, 1, 2048, theMP3File);
	for(x=0;x<0x2000;x++)
	{
//MP3_Fill_FIFO
		for(y=0;y<0x800;y++)
		{
			POKE(VS1053_5, PEEK(0xA000 + x));
			x++;
		}
//MP3_Fill_FIFO_Wait
		y=0;
		while((PEEK(VS1053_4) & 0x80) != 0x80) //FIFO_COUNT_HI
		{
			asm("NOP"); //wait for the fifo to empty itself
		}
		// ???
	
		POKE(0x0D,0x05);
	}

	POKE(1,0);
}
int main(int argc, char *argv[]) {

	setup();
	POKE(1,0);
 
	while(true)
        {
		kernelNextEvent();
        if(kernelEventData.type == kernelEvent(timer.EXPIRED))
            {
			switch(kernelEventData.timer.cookie)
				{
				case TIMER_TEXT_COOKIE:
					refreshPrints();
					textTimer.absolute = getTimerAbsolute(TIMER_FRAMES) + TIMER_TEXT_DELAY;
					setTimer(&textTimer);
					break;
				}
            }
		else if(kernelEventData.type == kernelEvent(key.PRESSED))
			{
				switch(kernelEventData.key.raw)
				{
					case 32: //space
						StefExample();
						break;
				}
				textGotoXY(40,20);
				printf(" %d  ",kernelEventData.key.raw);
			}
	
        }
return 0;}
