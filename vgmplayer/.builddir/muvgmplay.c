#include "D:\F256\llvm-mos\code\vgmplayer\.builddir\trampoline.h"

#include "f256lib.h"
#include "../src/muTimer0Int.h"  //timer0 routines with an interval of max 2/3 of a second
#include "../src/muopl3.h" //opl3 chip routines

uint32_t tooBigWait = 0x00000000; //keeps track of too big delays between events between vgmplay loop dips
bool comeRightTrough; //lets a chain of 0 delay events happen when returning to a new passthrough of playback()

//Opens the std VGM file
FILE *load_VGM_file(char *name) {
	FILE *theVGMfile;
	
	theVGMfile = fileOpen(name,"r"); // open file in read mode
	if(theVGMfile == NULL) {
		return NULL;
		}
	return theVGMfile;
}
uint8_t extraLines(uint16_t ver)
{
	switch(ver)
	{
		case 0x0100 ... 0x0101:
			return 0;
		case 0x0102 ... 0x0150:
			return 4;
		case 0x0151 ... 0x0160:
			return 12;
		case 0x0161 ... 0x0170:
			return 14;
		case 0x0171 ... 0x180:
			return 28;
	}
}


uint8_t checkVGMHeader(FILE *theVGMfile)
{
	uint8_t buffer[16];
	size_t bytesRead = 0;
	uint8_t extras =0;
	uint16_t version=0;
	bool isOPL3 = false;
	uint8_t dataOffset = 0x40;
	
	
	for(uint8_t i = 0; i<8+ extras; i++)
	{
	bytesRead = fileRead(buffer, sizeof(uint8_t), 16, theVGMfile );
	if(i==0) {
		version = (uint16_t)(buffer[0x9]<<8) | (uint16_t)buffer[0x8];
		extras = extraLines(version);
		}
	if(i==3) dataOffset = buffer[4] + 0x34;
	//for(uint8_t j = 0;j<16;j++) printf("%02x ", buffer[j]);
	if(i==5)
		{
		if((buffer[0] !=0 || buffer[1] !=0 || buffer[2] !=0 || buffer[3] !=0)) //YM3812 clock detection
			opl3_write(0x105,0); //set the chip in opl2 mode
	
		if((buffer[12] !=0 || buffer[13] !=0 || buffer[14] !=0 || buffer[15] !=0)) { //YMF262 clock detection
			opl3_write(0x105,1); //set the chip in opl3 mode
			isOPL3 = true;
			}
	
		textGotoXY(0,2);textSetColor(7,0);printf("Using v%04x %s",version, isOPL3?"OPL3":"OPL2");
		}
	}

return dataOffset;


}

int8_t playback(FILE *theVGMfile)
{
	uint8_t nextRead, countRead=0;
	uint8_t reg, val;
	uint8_t regval[2];
	uint8_t hi, lo;
	uint8_t exitFlag = 0;
	
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
			countRead = fileRead(&nextRead, sizeof(uint8_t), 1, theVGMfile);
	
			if (countRead == 1) {
				switch (nextRead) {
					case 0x5A:  // YMF3812 (opl2) write
						fileRead(&regval, sizeof(uint8_t), 2, theVGMfile);
						//fileRead(&val, sizeof(uint8_t), 1, theVGMfile);
						opl3_write(regval[0], regval[1]);
						//opl3_shadow(reg,val,0);
						comeRightTrough = true;
						//setTimer0(1);
						break;
					case 0x5E:  // YMF262 write port 0
						fileRead(&regval, sizeof(uint8_t), 2, theVGMfile);
						//fileRead(&val, sizeof(uint8_t), 1, theVGMfile);
						opl3_write(regval[0], regval[1]);
						//opl3_shadow(reg,val,0);
						comeRightTrough = true;
						//setTimer0(1);
						break;
	
					case 0x5F:  // YMF262 write port 1
						fileRead(&regval, sizeof(uint8_t), 2, theVGMfile);
						//fileRead(&val, sizeof(uint8_t), 1, theVGMfile);
						opl3_write((0x100 | (uint16_t)regval[0]), regval[1]);
						//opl3_shadow(reg,val,1);
						comeRightTrough = true;
						//setTimer0(1);
						break;
	
					case 0x61:  // Wait n samples
						fileRead(&lo, sizeof(uint8_t), 1, theVGMfile);
						fileRead(&hi, sizeof(uint8_t), 1, theVGMfile);
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
						break;
	
					case 0x63:
						setTimer0(0x7aE15);  // wait 1/50th of a second
						break;
					case 0x66: // End of sound data
						exitFlag = 1;
						return exitFlag;

					case 0x31: //skip one data byte commands
					case 0x4F:
					case 0x50:
						fileRead(&reg, sizeof(uint8_t), 1, theVGMfile);
						comeRightTrough = true;
						//setTimer0(1);
						break;
	
					case 0x40: //skip two data bytes commands
					case 0x51 ... 0x59:
					case 0x5B ... 0x5D:
					case 0xA0:
					case 0xB0 ... 0xC8:
						fileRead(&reg, sizeof(uint8_t), 1, theVGMfile);
						fileRead(&reg, sizeof(uint8_t), 1, theVGMfile);
						comeRightTrough = true;
						//setTimer0(1);
						break;
	
					case 0xD0 ... 0xD6: //skip 3 data bytes commands
						fileRead(&reg, sizeof(uint8_t), 1, theVGMfile);
						fileRead(&reg, sizeof(uint8_t), 1, theVGMfile);
						fileRead(&reg, sizeof(uint8_t), 1, theVGMfile);
						comeRightTrough = true;
						//setTimer0(1);
						break;
					default:
						comeRightTrough = true;
						//setTimer0(1);
						break;
					}
				}
			else return 0;
			}
		}
	
	return 0;
}

