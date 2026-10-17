#include "D:\F256\llvm-mos\code\f256amp\.builddir\trampoline.h"

#define F256LIB_IMPLEMENTATION
#include "f256lib.h"

#include "../src/muUtils.h"
#include "../src/muVS1053b.h"
#include "../src/muFilePicker.h"
#include "../src/muTextUI.h" //text dialogs and file directory and file picking
#include "../src/muFileDiplomacy.h"
#include "../src/muMP3Play.h"
#include "../src/muTimer0Int.h"

#define LCD_BAND_WIDTH    2
#define LCD_BAND_INTERV   8
#define LCD_BAND_HEIGHT   61
#define LCD_BAND_START_Y  40
#define LCD_BAND_START_X  60
#define LCD_BAND_AREA     (LCD_BAND_HEIGHT * LCD_BAND_WIDTH)

#define NB_BANDS 15 //use 15 for new patch, 14 for no patch


#define LCD_CMD_CMD 0xDD40  //Write Command Here
#define LCD_RST 0x10        //0 to Reset (RSTn)
#define LCD_BL  0x20        //1 = ON, 0 = OFF
#define LCD_WIN_X 0x2A    // window X command
#define LCD_WIN_Y 0x2B    // window Y command
#define LCD_WRI   0x2C    //write command
#define LCD_RD    0x2E    //read  command
#define LCD_MAD   0x36    //MADCTL control register
#define LCD_TE  0x40        //Tear Enable (avoid updating the display while the controller reads its value)

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


#define LCDBIN 0x20000
#define LOGOPAL 0x43000
#define LOGOBIN 0x43400
#define VSPATCH 0x56000
#define VSPLUGIN 0x59000

//main color text LUT setup
char finalName[64];

uint8_t openMP3File(char *);
void setTextLUT1(void);
void setTextLUT2(void);
void backgroundSetup(void);
bool K2LCD(void);
bool wrongMachineTest(void);


void clearVisible(uint16_t);
void displayImage(uint32_t);
void prepareRect(uint8_t, uint16_t, uint8_t, uint16_t);
void setLCDReverseY(void);

EMBED(mac, "../assets/f256amp.bin", 0x20000);
EMBED(allo, "../assets/mainlogo.raw.pal", 0x43000);
EMBED(popo, "../assets/mainlogo.raw", 0x43400);
EMBED(patch, "../assets/bigpatch.bin", 0x56000);
EMBED(saplug, "../assets/saplugin2.bin", 0x59000);


//main color text LUT setup
void setTextLUT1() {
	uint16_t c=0;
	POKE(MMU_IO_CTRL,0);
	for(c=0;c<8;c++)
	{
		POKE(0xD800+c*4,0);
		POKE(0xD801+c*4,0xFF);
		POKE(0xD802+c*4,0+(c<<5));
	}
	for(c=8;c<16;c++)
	{
		POKE(0xD800+c*4,0);
		POKE(0xD801+c*4,0xFF-((c-8)<<5));
		POKE(0xD802+c*4,0xFF);
	}
}

//secondary color text LUT setup
void setTextLUT2() {
	uint16_t c=0;
	POKE(MMU_IO_CTRL,0);
	for(c=0;c<8;c++)
		{
		POKE(0xD800+c*4,0xFF);
		POKE(0xD801+c*4,0+(c<<5));
		POKE(0xD802+c*4,0);
		}
	for(c=8;c<16;c++)
		{
		POKE(0xD800+c*4,0xFF-((c-8)<<5));
		POKE(0xD801+c*4,0xFF);
		POKE(0xD802+c*4,0);
		}
}
void colMatrixSetup()
{
		//color text matrix setup
	POKE(MMU_IO_CTRL,3);
	for(uint16_t h=0;h<20;h++)
		{
		for(uint16_t c=0;c<80;c++)
			{
			POKE(0xC910+c-h*80,0x0F+h*12);
			}
		}
	POKE(MMU_IO_CTRL, 0x00);
}

void backgroundSetup() {
	uint16_t c=0;
	
	POKE(MMU_IO_CTRL, 0);
	// XXX GAMMA  SPRITE   TILE  | BITMAP  GRAPH  OVRLY  TEXT
	POKE(VKY_MSTR_CTRL_0, 0b00101111); //sprite,graph,overlay,text
	// XXX XXX  FON_SET FON_OVLY | MON_SLP DBL_Y  DBL_X  CLK_70
	POKE(VKY_MSTR_CTRL_1, 0b00000100); //font overlay, double height text, 320x240 at 60 Hz;
	POKE(VKY_LAYER_CTRL_0, 0b00000001); //bitmap 1 in layer 0, bitmap 0 in layer 1
	POKE(VKY_LAYER_CTRL_1, 0b00000010); //bitmap 2 in layer 2
	POKE(0xD00D,0x00); //force black graphics background
	POKE(0xD00E,0x00);
	POKE(0xD00F,0x00);

//bitmap CLUT
	POKE(MMU_IO_CTRL,1);  //MMU I/O to page 1
	// Set up CLUT0.
	for(c=0;c<1023;c++)
	{
		POKE(VKY_GR_CLUT_0+c, FAR_PEEK(LOGOPAL+c));
	}
	POKE(MMU_IO_CTRL, 0);
	
	bitmapSetActive(0);
	
	bitmapSetAddress(0,LOGOBIN);
	bitmapSetCLUT(0);
	
	bitmapSetVisible(0,true);
	bitmapSetVisible(1,false);
	bitmapSetVisible(2,false);
	
	setTextLUT1();
	colMatrixSetup();

}

bool K2LCD() {
	if(isK2())
	{
	displayImage(LCDBIN);
	return true;
	}
	else return false;
}

bool wrongMachineTest() {

	if(isWave2() == false)
		{

		uint8_t mid = PEEK(0xD6A7);
		printf("Your machine ID is %02x.",mid);
		printf("\nIn order to work, a VS1053b chip needs to be present.");
		printf("\nThis chip is present in the Jr2 and K2.");
		printf("\nHit space if you think you have it.");
		hitspace();

		return false;
		}

	return true;
}

//clears the screen with a R5G6B6 word containing the color data
void clearVisible(uint16_t colorWord) {
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


//will display a 240x280 image centered on the screen. This assumes a R5G6B5 bitmap file
//converted to a 2 byte per pixel raw binary file, that will be sent to LCD_PIX_LO and LCD_PIX_HI
//in that order, pixel by pixel
//look for my instructions in https://wiki.f256foenix.com/index.php?title=Use_the_K2_LCD
void displayImage(uint32_t addr) {
	uint32_t i;
	uint32_t j;
	uint32_t index = 0;
	POKE(LCD_CMD_CMD, LCD_WIN_X);
	POKE(LCD_CMD_DTA, 0); //xstart high
	POKE(LCD_CMD_DTA, 0); //xstart low
	POKE(LCD_CMD_DTA, 0); //xend high
	POKE(LCD_CMD_DTA, 239); //xend low

	POKE(LCD_CMD_CMD, LCD_WIN_Y);
	POKE(LCD_CMD_DTA, 0); //ystart high
	POKE(LCD_CMD_DTA, 20); //ystart low
	POKE(LCD_CMD_DTA, 0x01); //yend high
	POKE(LCD_CMD_DTA, 0x3F); //yend low
	
	POKE(LCD_CMD_CMD, LCD_WRI);
	
	for(j=0;j<280;j++)
	{
		for(i=0;i<240;i++)
		{
			POKE(LCD_PIX_LO, FAR_PEEK(addr + index));
			POKE(LCD_PIX_HI, FAR_PEEK(addr + index + (uint32_t)1));
			index+=2;
		}
	}
}

//sets the next rectangle region to be written at coordinates x,y on the LCD
void prepareRect(uint8_t x, uint16_t y, uint8_t width, uint16_t height) {
	POKE(LCD_CMD_CMD, LCD_WIN_X);
	POKE(LCD_CMD_DTA, 0); //xstart high
	POKE(LCD_CMD_DTA, x); //xstart low
	POKE(LCD_CMD_DTA, 0); //xend high
	POKE(LCD_CMD_DTA, x+width-1); //xend low
	
	POKE(LCD_CMD_CMD, LCD_WIN_Y);
	POKE(LCD_CMD_DTA, (y&0xFF00)); //ystart high
	POKE(LCD_CMD_DTA, 20 + (y&0x00FF)); //ystart low
	POKE(LCD_CMD_DTA, (y+height)&0x00FF); //yend high
	POKE(LCD_CMD_DTA, (y+height)&0x00FF-1); //yend low
	
	POKE(LCD_CMD_CMD, LCD_WRI);
}
	

void setLCDReverseY() {
	POKE(LCD_CMD_CMD, LCD_MAD);
	POKE(LCD_CMD_DTA, 0x80);
	
	POKE(LCD_CMD_CMD, LCD_WRI);
}

void writeTopRight(bool wantCmds)
{
	textSetColor(5,0);
	textGotoXY(66,1);textPrint("F256Amp v2.4");
	textGotoXY(67,2);textPrint("October 2026");
	textGotoXY(71,3);textPrint("by Mu0n");
	textGotoXY(62,8);textPrint("[ESC]Quit");
	
	if(wantCmds)
	{
	textGotoXY(0,0);
	printf("Currently playing %s                               ",name);
	textGotoXY(62,9);textPrint("[F1]Color Change");
	textGotoXY(62,10);textPrint("[F3]Load File");
	}
	else
	{
	textGotoXY(0,0);
	printf("                                                                                ");
	textGotoXY(62,9);textPrint("                ");
	textGotoXY(62,10);textPrint("             ");
	}
}
int main(int argc, char *argv[]) {
	(void)argc;
	(void)argv;
	uint16_t i=0,j=0;
	uint16_t saVals[NB_BANDS];
	uint16_t oldVals[NB_BANDS];
	uint16_t charSABase = 0xC921;
	uint8_t savedIO;
	uint16_t peak;
	bool startHear = false; //gives permission to do spectrum analysis bars when the sound gets going
	bool colorToggler = false;
	bool greenLCD = true; //send SA to LCD
	bool greenScreen = true; //send SA to Main screen
	uint16_t lcdStartY=0;
	bool firstTime = true; //first time it loads, next times it keeps the cursor position
	bool cliFile = false; //first time it loads, next times it keeps the cursor position
	bool finished = false;
	

    backgroundSetup();
	wrongMachineTest(); //test wave 2

	greenLCD = K2LCD(); //if false, never try to write to LCD, on a Jr2, if true will display the initial image

lilpause(1);
	if(greenLCD == true) setLCDReverseY();

	initFPR();
	
	openAllCODEC();
	boostVSClock();
	writeTopRight(false);

	
    initBigPatch(VSPATCH);
	initSpectrum(VSPLUGIN);
	setTimer1(1);
	kqReset();
	
for(;;) {
	//prep stuff and re-prep if loading after the first file
	finished=false;
	startHear=false;
	if(colorToggler == false) setTextLUT1();
	else setTextLUT2();
	colMatrixSetup();
	
	for(i=0;i<NB_BANDS;i++)
		{
		saVals[i]=0;
		oldVals[i]=0;
		}
	
	textGotoXY(0,0);
	if(cliFile == false) if(getTheFile_far(name) == 1) return 0; //pick a file if we didn't get one from a cli parameter
	
	
	firstTime = false;
	writeTopRight(false);
	
	textGotoXY(0,0);
	printf("Currently loading %s                               ",name);
    if(colorToggler == false) setTextLUT1();
	else setTextLUT2();
	colMatrixSetup();

	if(prepMP3ForPlay(name)) printf("error loading file");
	
	
	cliFile = false; //never skip this for next playbacks

	while(true) //playback is occuring
		{
	
		//when a non-zero value is detected for the first time, mark that it's now playing.
		if(startHear==false)
			{
			if((saVals[2]&31)!=0)
				{
				writeTopRight(true);
				startHear = true;
				}
			}
	
		//keyboard presses
		while(pullEventB())
		{
		if(kernelEventData.type == kernelEvent(key.PRESSED))
			{
				switch(kernelEventData.key.raw)
				{
				case 146: //esc
					closeDownMp3File();
					return 0;
				case 129: //F1
					if(colorToggler)
						{
						setTextLUT1();
						colorToggler = false;
						}
					else
						{
						setTextLUT2();
						colorToggler = true;
						}
					break;
				case 0x83: //F3
					finished=true;
					break;
				}
			}
		}
		//Visuals tied to time1, related to frames 60Hz
	
		if(PEEK(INT_PENDING_0)&0x20) //when the timer1 delay is up, go here
		{
		POKE(INT_PENDING_0,0x20); //clear pending timer1
	
	
		getCenterSAValues(NB_BANDS, saVals);
	
	
		for(j=1;j<NB_BANDS;j++)
			{
			peak=(saVals[j]&31);
	
			//LCD spectrum analyzer
			if(greenLCD == true)
				{
				if(oldVals[j] > peak) //band is reducing, write red over the reduction only
					{
					lcdStartY = LCD_BAND_START_Y + (peak<<1);
	
					prepareRect(LCD_BAND_START_X + LCD_BAND_INTERV*j, lcdStartY, LCD_BAND_WIDTH,oldVals[j]-peak);
	
					for(i=0;i<=(LCD_BAND_WIDTH * ((oldVals[j]-peak)<<1));i++)
						{
						POKEW(LCD_PIX_LO,LCD_PURE_RED);
						}
					}
				else if(peak > oldVals[j]) //band is growing, write green over the augmentation only
					{
					lcdStartY =  LCD_BAND_START_Y + (oldVals[j]<<1);
	
					prepareRect(LCD_BAND_START_X + LCD_BAND_INTERV*j, lcdStartY, LCD_BAND_WIDTH,peak-oldVals[j]);
	
					for(i=0;i<=(LCD_BAND_WIDTH * ((peak-oldVals[j])<<1));i++)
						{
						POKEW(LCD_PIX_LO,LCD_PURE_GRN);
						}
					}
				}

			//main screen text matrix spectrum analyzer
			if(greenScreen)
				{
				savedIO = PEEK(MMU_IO_CTRL);
				POKE(MMU_IO_CTRL,2);
				if(peak > oldVals[j]) //only draw if longer green bars are needed
					{
					for(i=oldVals[j];i<peak;i++)
						{
						POKE(charSABase-i*80+3*j,0x15);
						POKE(charSABase-i*80+3*j+1,0x15);
						}
					}
				else if(peak < oldVals[j]) //only erase if shorter bars were needed
					{
					for(i=peak;i<24;i++)
						{
							POKE(charSABase-i*80+3*j,0x20);
							POKE(charSABase-i*80+3*j+1,0x20);
						}
					}
				POKE(MMU_IO_CTRL,savedIO);
				}
	
				oldVals[j] = peak;
			}
		setTimer1(1); //pump it back and establish it at 30 fps
		}
	

		//Check the health of the FIFO buffer
	
		if(MP3LoopPass()) break;
		 /*
		 textGotoXY(0,23);
   printf("t=%02x r=%02x m=%02x p=%02x h=%u t=%u  ",
          kernelEventData.type, kernelEventData.key.raw,
          PEEK(INT_MASK_0), PEEK(INT_PENDING_0), kqHead, kqTail);
		*/

		if(finished) break;
		} //playback loop

	closeDownMp3File();
	
}//end main loop
	return 0;
}//end main
