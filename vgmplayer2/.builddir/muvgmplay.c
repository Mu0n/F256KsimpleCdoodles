#include "D:\F256\llvm-mos\code\vgmplayer2\.builddir\trampoline.h"

#include "f256lib.h"
#include "../src/muTimer0Int.h"  //timer0 routines with an interval of max 2/3 of a second
#include "../src/muopl3.h" //opl3 chip routines
#include "../src/muUtils.h" //useful routines
#include "../src/muvgmplay.h" //useful routines
#include "../src/textUI.h"

uint32_t tooBigWait = 0x00000000; //keeps track of too big delays between events between vgmplay loop dips
bool comeRightTrough; //lets a chain of 0 delay events happen when returning to a new passthrough of playback()
uint32_t needle; //pointer to high ram available in 2x only.
uint32_t totalWait; //wait samples to figure out end of song
uint32_t loopBackTo; //loopback to this position
uint32_t samplesSoFar; //done samples so far, to trigger end of song
uint32_t gd3Location; //gd3 tag so we can ignore iteration

bool oneLoop = false; //for songs that have a loop, set this once and do one loop, then finish the song. hybrid approach for jukeboxing and authenticity

//Opens the std VGM file
//__attribute__((optnone))
FILE *load_VGM_file(char *name) {
	FILE *theVGMfile;
	
	theVGMfile = fileOpen(name,"r"); // open file in read mode
	if(theVGMfile == NULL) {
		return NULL;
		}
	return theVGMfile;
}
uint8_t getStart(uint16_t ver)
{
	if(ver < 0x150) return 0x40;
	else return 0x00;

}

__attribute__((optnone))
void copyToRAM(FILE *theVGMfile)
{
	bool exitFlag = true;
	char buffer[255];
	uint8_t bytesRead = 0;
	uint32_t soFar = 0;
	
	while(exitFlag)
	{
		bytesRead = fileRead(buffer, sizeof(uint8_t), 255, theVGMfile);
		if(bytesRead != 255) exitFlag = false;
		for(uint8_t i=0; i<bytesRead; i++)
			{
			POKE24(VGM_BODY+(uint32_t)i+(uint32_t)soFar, buffer[i]);
			}
		soFar+= bytesRead;
	}
	/*
	bitmapSetVisible(0,false);
	textGotoXY(0,0);
	for(uint8_t i =0; i<25; i++)
	{
		for(uint8_t j=0;j<16; j++)
		{
			printf("%02x",PEEK24(VGM_BODY+(uint32_t)j+((uint32_t)i*(uint32_t)16)));
		}
		printf("\n");
	}
	hitspace();
*/
}

void checkVGMHeader(FILE *theVGMfile)
{
	uint8_t buffer[16];
	uint16_t version=0;
	bool isOPL3 = false;
	uint8_t dataOffset=0x00;
	gd3Location = 0;
	oneLoop = false;
	
	for(uint8_t i = 0; i<8; i++)
		{
		fileRead(buffer, sizeof(uint8_t), 16, theVGMfile );
	
		if(i==0) {
			version = (uint16_t)(buffer[0x9]<<8) | (uint16_t)buffer[0x8];
			dataOffset = getStart(version);
			}
		if(i==1)
			{
			gd3Location = (uint32_t)buffer[0x4] | ((uint32_t)buffer[0x5])<<8 | ((uint32_t)buffer[6])<<16 | ((uint32_t)buffer[7])<<24;
			totalWait = (uint32_t)buffer[0x8] | ((uint32_t)buffer[0x9])<<8 | ((uint32_t)buffer[0xA])<<16 | ((uint32_t)buffer[0xB])<<24;
	
			loopBackTo = (uint32_t)buffer[0xC] | ((uint32_t)buffer[0xD])<<8 | ((uint32_t)buffer[0xE])<<16 | ((uint32_t)buffer[0xF])<<24;
			}
	
		if(i==3 && dataOffset == 0)
		{
			dataOffset = buffer[4]+0x34;
		}
		if(i==5)
			{
			if((buffer[0] !=0 || buffer[1] !=0 || buffer[2] !=0 || buffer[3] !=0))		//YM3812 clock detection
				opl3_write(0x105,0); //set the chip in opl2 mode
	
	
			if((buffer[12] !=0 || buffer[13] !=0 || buffer[14] !=0 || buffer[15] !=0)) //YMF262 clock detection
				{
				opl3_write(0x105,1); //set the chip in opl3 mode
				isOPL3 = true;
				}
			}
	
		}
	textGotoXY(0,2);textSetColor(7,0);printf("Using v%04x %s",version, isOPL3?"OPL3":"OPL2");
/*
	textGotoXY(0,3);printf("                                                                                ");
	textGotoXY(0,4);printf("                                                                                ");
	*/
	eraseLine(3);
	textGotoXY(0,3);textSetColor(7,0);printf("Loading, please wait");
	//copy to high ramhitspace();
	fileSeek(theVGMfile,(uint32_t)dataOffset, SEEK_SET);
	copyToRAM(theVGMfile);

	//header no, no longer need the file

	
	
	textGotoXY(0,3);textSetColor(7,0);printf("Playing:");
	needle = VGM_BODY;
	if(loopBackTo!=0) loopBackTo = VGM_BODY + (uint32_t)loopBackTo - (uint32_t)dataOffset;
	if(gd3Location != 0) gd3Location = VGM_BODY + gd3Location - (uint32_t)dataOffset + (uint32_t)0x14;
	//textGotoXY(40,0);printf("\ndata offset %02x, total %08lx loopBack %08lx gd3 %08lx",dataOffset, totalWait, loopBackTo, gd3Location);
	samplesSoFar=0;
	
		fileClose(theVGMfile);
	
}

int8_t playback(FILE *theVGMfile, bool iRT, bool pReq)
{
	uint8_t nextRead, countRead=0;
	uint8_t reg, val;
	uint8_t hi, lo;
	int8_t canPause = 0; //will only become true when a key on event is done
	
	hi=0;lo=0;


	if(PEEK(INT_PENDING_0)&0x10 || comeRightTrough == true) //when the timer0 delay is up, go here
		{
		if(comeRightTrough == false) POKE(INT_PENDING_0,0x10); //clear the timer0 delay
	
		if(tooBigWait > 0 && comeRightTrough == false)
			{
			if(tooBigWait > 0x00FFFFFF) //this is going to require a full 0.666 and potentially more
				{
				setTimer0(0x00FFFFFF);
				tooBigWait -= 0x00FFFFFF;
				return 0;
				}
			else
				{
				setTimer0(tooBigWait); //one last loop iteration
				tooBigWait = 0; //expire the rest
				return 0;
				}
			}
		else
			{
			comeRightTrough = false;
			//countRead = fileRead(&nextRead, sizeof(uint8_t), 1, theVGMfile);
	
			if((samplesSoFar >= totalWait || needle >= gd3Location) && gd3Location != 0)
			{
				if(loopBackTo == 0 || oneLoop == true) return -1; //end of file, there was no loop to do
				needle = loopBackTo; //loop is performed here
				samplesSoFar = 0;
				oneLoop = true;
			}
			nextRead = PEEK24(needle++);
			countRead = 1; //bypass old file read check
	
			//if(readDebug<40)textGotoXY(0,readDebug++);printf("command %02x",nextRead);
	
	
			if (countRead == 1) {
				switch (nextRead) {
					case 0x5A:  // YMF3812 (opl2) write
						//fileRead(&regval, sizeof(uint8_t), 2, theVGMfile);
						reg = PEEK24(needle++);
						val = PEEK24(needle++);
						//fileRead(&val, sizeof(uint8_t), 1, theVGMfile);
						opl3_write(reg, val);
						if(opl3_shadow(reg,val,0,iRT)==1) canPause = -2;
						comeRightTrough = true;
						//setTimer0(1);
						break;
					case 0x5E:  // YMF262 write port 0
						//fileRead(&regval, sizeof(uint8_t), 2, theVGMfile);
						reg = PEEK24(needle++);
						val = PEEK24(needle++);
						//fileRead(&val, sizeof(uint8_t), 1, theVGMfile);
						opl3_write(reg, val);
						if(opl3_shadow(reg,val,0,iRT)==1) canPause = -2;
						comeRightTrough = true;
						//setTimer0(1);
						break;
	
					case 0x5F:  // YMF262 write port 1
						//fileRead(&regval, sizeof(uint8_t), 2, theVGMfile);
						reg = PEEK24(needle++);
						val = PEEK24(needle++);
						//fileRead(&val, sizeof(uint8_t), 1, theVGMfile);
						opl3_write((0x100 | (uint16_t)reg), val);
						if(opl3_shadow(reg,val,1,iRT)==1) canPause = -2;
						comeRightTrough = true;
						//setTimer0(1);
						break;
	
					case 0x61:  // Wait n samples
						lo = PEEK24(needle++);
						hi = PEEK24(needle++);
	
						samplesSoFar+=(uint32_t)lo | ((uint32_t)hi)<<8;
						//fileRead(&lo, sizeof(uint8_t), 1, theVGMfile);
						//fileRead(&hi, sizeof(uint8_t), 1, theVGMfile);
						tooBigWait = ((((uint32_t)hi)<<8)|((uint32_t)lo))*(uint32_t)0x23A;
	
						//tooBigWait = mathUnsignedMultiply( (((uint16_t)hi)<<8)|((uint16_t)lo), 256);
						if(tooBigWait > 0x00FFFFFF) //this is going to require a full 0.666s and potentially more
							{
							setTimer0(0x00FFFFFF);
							tooBigWait -= 0x00FFFFFF;
							}
						else //this is under 0.666s
							{
							setTimer0(tooBigWait);
							tooBigWait = 0;
							}
						break;

					case 0x62:
						setTimer0(0x66666);   // wait 1/60 of a second
						samplesSoFar+=735;
						break;
	
					case 0x63:
						setTimer0(0x7aE15);  // wait 1/50th of a second
						samplesSoFar+=882;
						break;
					case 0x66: // End of sound data
						return -1;
						if(loopBackTo == 0 && oneLoop == true) return -1;
						else if(loopBackTo != 0) oneLoop = true;
						comeRightTrough = true;
						break;
					case 0x67: // data block
						needle+=2;
						uint32_t skippy = (PEEK24(needle)) | (PEEK24(needle+1)<<8) | (PEEK24(needle+2)<<16) | (PEEK24(needle+3)<<24);
						needle+=4+skippy;
						break;
					case 0x30 ... 0x3F: //skip one data byte commands
					case 0x4F:
					case 0x50:
						//fileRead(&reg, sizeof(uint8_t), 1, theVGMfile);
						needle++;
	
						comeRightTrough = true;
						//setTimer0(1);
						break;
	
					case 0x40 ... 0x4E: //skip two data bytes commands
					case 0x51 ... 0x59:
					case 0x5B ... 0x5D:
					case 0xA0:
					case 0xB0 ... 0xC8:
	
						needle+=2;
						//fileRead(&reg, sizeof(uint8_t), 1, theVGMfile);
						//fileRead(&reg, sizeof(uint8_t), 1, theVGMfile);
						comeRightTrough = true;
						//setTimer0(1);
						break;
	
					case 0xD0 ... 0xDF: //skip 3 data bytes commands
					case 0xC9 ... 0xCF:
						needle+=3;
						//fileRead(&reg, sizeof(uint8_t), 1, theVGMfile);
						//fileRead(&reg, sizeof(uint8_t), 1, theVGMfile);
						//fileRead(&reg, sizeof(uint8_t), 1, theVGMfile);
						comeRightTrough = true;
						//setTimer0(1);
						break;


					case 0xE2 ... 0xFF://skip 4 data bytes commands
						needle+=4;
						comeRightTrough = true;
						break;
					default:
					    //printf("unsupported command %02x\n",nextRead);
						comeRightTrough = true;
						//setTimer0(1);
						break;
					}
				}
			}
		}
	if(pReq) return canPause;
	else return 0;
}

