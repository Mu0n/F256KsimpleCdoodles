#define F256LIB_IMPLEMENTATION
#include "f256lib.h"
#include "../src/muUtils.h"
#include "../src/muVS1053b.h"
#include "../src/mulcd.h"
#include "../src/muFilePicker.h"
#include "../src/muTextUI.h" //text dialogs and file directory and file picking

#define CHUNK8K 0x2000
#define CHUNK4K 0x1000
#define CHUNK2K 0x0800
#define CHUNK1K 0x0400
#define CHUNK128B 0x80
#define CHUNK64B 0x40
#define CHUNK32B 0x20

#define DIRECTORY_X 1
#define DIRECTORY_Y 7

#define LCD_BAND_WIDTH    2
#define LCD_BAND_INTERV   8
#define LCD_BAND_HEIGHT   61
#define LCD_BAND_START_Y  40
#define LCD_BAND_START_X  60
#define LCD_BAND_AREA     (LCD_BAND_HEIGHT * LCD_BAND_WIDTH)

#define NB_BANDS 15 //use 15 for new patch, 14 for no patch

#define LCDBIN 0x20000
#define LOGOPAL 0x53000
#define LOGOBIN 0x53400



EMBED(mac, "../assets/f256amp.bin", 0x20000);
EMBED(pally, "../assets/mainlogo.raw.pal", 0x52000);
EMBED(logoy, "../assets/mainlogo.raw", 0x52400);
EMBED(patch, "../assets/bigpatch.bin", 0x66000);
EMBED(saplug, "../assets/saplugin2.bin", 0x68476);

void read8KChunk(uint8_t *, FILE *);
uint8_t openMP3File(char *);
void setTextLUT1(void);
void setTextLUT2(void);
void backgroundSetup(void);
bool K2LCD(void);
bool wrongMachineTest(void);


FILE *theMP3file;
filePickRecord fpr;
char finalName[64];
char *queryFudge[] = {
"                                                       ",
"       Choose an audio file for playback               ", 
" mp3, wav, ogg, wma formats are supported              ", 
"         The default directory is 0:mp3/               ",
"                                                       "
};


uint8_t openMP3File(char *name) {
	theMP3file = fileOpen(name,"rb"); // open file in read mode
	if(theMP3file == NULL)
	{
		return 1;
	}
	return 0;
}

void read8KChunk(uint8_t *buf, FILE *f) {
	uint16_t i;
	for(i=0;i<64;i++)
		{
		fileRead((buf+i*0x80), sizeof(uint8_t), 128, f); //read 128 bytes at a time, since there's a hard limit of 255 reads at a time. 64x128 = 8k = 8192 bytes
		}
}

//main color text LUT setup
void setTextLUT1() {
	uint16_t c=0;
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
	
void backgroundSetup() {
	uint16_t c=0;
	uint16_t h=0;
	
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
	
	//color text matrix setup
	POKE(MMU_IO_CTRL,3);
	for(h=0;h<20;h++)
		{
		for(c=0;c<80;c++)
			{
			POKE(0xC910+c-h*80,0x0F+h*12);
			}
		}
	POKE(MMU_IO_CTRL, 0x00);
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
		printf("In order to work, a VS1053b chip needs to be present.\nOnly for the K2 and Jr.2");
		printf("\nHit space to quit.");
		hitspace();
		return false;
		}
	return true;
}

int main(int argc, char *argv[]) {	
	(void)argc;
	(void)argv;
	uint16_t i=0,j=0;
	uint16_t bufferIndex=0; //index for the local 8k buffer register
    uint16_t readEntryBufferIndex = 0;
	uint16_t rawFIFOCount=0;
	uint16_t bytesToTopOff=0;
	uint16_t multipleOf64b = 0;
	uint16_t saVals[NB_BANDS];
	uint16_t oldVals[NB_BANDS];
	uint16_t charSABase = 0xC921;
	uint8_t savedIO;
	uint16_t peak;
	uint8_t deliveredBytes=0;
	bool finished = false;
	uint16_t endZone = 2048;
	bool startHear = false;
	bool colorToggler = false;
	bool greenLCD = true; //send SA to LCD 
	bool greenScreen = true; //send SA to Main screen
	uint16_t lcdStartY=0;
	uint8_t storedRawKey=0;
	bool dealWithStoredKey = false;

	uint8_t buffer[CHUNK8K]; //4x the size of the VS FIFO buffer
	char zeroes[64]; //0's at the end
	
	for(i=0;i<NB_BANDS;i++) 
		{
		saVals[i]=0;
		oldVals[i]=0;
		}
	for(i=0;i<64;i++) zeroes[i]=0;
	
    backgroundSetup();
	if(wrongMachineTest() == false) return 0; //immediately quit
	/*
	greenLCD = K2LCD(); //if false, never try to write to LCD, on a Jr2, if true will display the initial image
	if(greenLCD == true) setLCDReverseY();
	*/
	
    filePickModal(&fpr, DIRECTORY_X, DIRECTORY_Y, "mp3", "mp3", "wav", "ogg", "wma");

	sprintf(finalName, "%s%s%s%s", "0:", fpr.currentPath, "/",fpr.selectedFile);
	printf("final name %s", finalName);
	
	if(openMP3File(finalName)) printf("error loading file");
/*
	if(argc > 1)
	{
		i=0;
		if(argv[1][0] == '-')
			{
			filePickModal(&fpr);
			}
		else
			{
			while(argv[1][i] != '\0')
				{
					fileName[i] = argv[1][i];
					i++;
				}		

			fileName[i] = '\0';
			}
	}
	else filePickModal(&fpr);
*/

	openAllCODEC();
	boostVSClock();
    initBigPatch();
	initSpectrum();
	printf("before reading 8K chunk");
	read8KChunk(buffer, theMP3file); //read the first 8k chunk from the .mp3 file	

	printf("after reading 8K chunk");
	textSetColor(5,0);
	textGotoXY(66,1);textPrint("F256Amp v1.0");
	textGotoXY(68,2);textPrint("July 2025");
	textGotoXY(71,3);textPrint("by Mu0n");
	
//	textGotoXY(0,0);printf("Loading %s ...",currentFilePicked);

	for(i=bufferIndex;i<bufferIndex+CHUNK2K;i++) //fill the first 2k chunk into the full size of the buffer
		{
		POKE(VS_FIFO_DATA, buffer[i]);
		}
	bufferIndex+=CHUNK2K;
	for(i=bufferIndex;i<bufferIndex+CHUNK2K;i++) //fill the first 2k chunk into the full size of the buffer
		{
		POKE(VS_FIFO_DATA, buffer[i]);
		}
	bufferIndex+=CHUNK2K;
	
	while(true)
		{
			
		//getCenterSAValues(NB_BANDS, saVals);
		//when a non-zero value is detected for the first time, mark that it's now playing.
		if(startHear==false)
			{
			if((saVals[2]&31)!=0)
				{
				textGotoXY(0,0);
//				printf("Currently playing %s       ",currentFilePicked);
				textGotoXY(62,8);textPrint("[F1]Color Change");
				startHear = true;
				}
			}
		
		//keyboard presses
		kernelNextEvent();
		if(kernelEventData.type == kernelEvent(key.PRESSED))
			{
				switch(kernelEventData.key.raw)
				{
				case 146: //esc
					fileClose(theMP3file);
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
				}
			}
		if(dealWithStoredKey)
			{
			switch(storedRawKey)
				{
				case 146: //esc
					fileClose(theMP3file);
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
					dealWithStoredKey=false;
					break;
				}
			}	
			
		//Visuals
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
		
			

		//Check the health of the FIFO buffer
		rawFIFOCount = PEEKW(VS_FIFO_COUNT);
		bytesToTopOff = CHUNK2K - (rawFIFOCount&0x07FF); //found how many bytes are left in the 2KB buffer
		multipleOf64b = bytesToTopOff>>6; //multiples of 64 bytes of stuff to top off the FIFO buffer

		for(i=0; i<multipleOf64b; i++)
			{
			if(finished) break;			//make sure we intercept key presses during copying
			if(dealWithStoredKey == false)
			{
				kernelNextEvent();
				if(kernelEventData.type == kernelEvent(key.PRESSED) && dealWithStoredKey == false)
					{
					dealWithStoredKey = true;
					storedRawKey=kernelEventData.key.raw;
					}
			}
			
			//Copy a chunk from the application buffer to the VS' buffer
			for(j=0; j<CHUNK64B; j++) {
				POKE(VS_FIFO_DATA, buffer[bufferIndex+j]);
				}

			//advance the application buffer index
			bufferIndex+=CHUNK64B;
			if(bufferIndex == CHUNK8K) bufferIndex = 0; //warp over if the 8KB limit is reached
			
			//read more of the file to replace an equivalent chunk to the application buffer
			deliveredBytes = fread((void *)(buffer+readEntryBufferIndex), sizeof(uint8_t), 64, theMP3file); //Check if we're getting to the end of the file
			if(deliveredBytes !=64) 
				{
				finished = true;
				readEntryBufferIndex+=deliveredBytes; 
				}
			else 
				{
				readEntryBufferIndex+=CHUNK64B; //keep track of where we're at so we can finish the file
				if(readEntryBufferIndex== CHUNK8K) readEntryBufferIndex = 0; //keep track of where we're at so we can finish the file
				}
			}
		if(finished) break;
		}
	//end is reached, gracefully fill the buffer with 0's now.
	while(endZone !=0)
		{
		rawFIFOCount = PEEKW(VS_FIFO_COUNT);
		bytesToTopOff = CHUNK2K - (rawFIFOCount&0x07FF); //found how many bytes are left in the 2KB buffer
		multipleOf64b = bytesToTopOff>>6; //multiples of 64 bytes of stuff to top off the FIFO buffer
		for(i=0; i<multipleOf64b; i++)
			{
			for(j=0; j<CHUNK64B; j++) POKE(VS_FIFO_DATA, zeroes[j]);
			endZone -= CHUNK64B;	
			}
		}

	fileClose(theMP3file);
	return 0;
}
}