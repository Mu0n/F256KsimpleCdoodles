#define F256LIB_IMPLEMENTATION

#include "f256lib.h"
#include "../src/muUtils.h" //contains helper functions I often use

int main(int argc, char *argv[]) {
	uint8_t *fileNum =0;
	uint8_t binData[]={0xDE,0xAD,0xBE,0xEF,0xDE,0xAD,0xBE,0xEF,
					   0xDE,0xAD,0xBE,0xEF,0xDE,0xAD,0xBE,0xEF};
	uint8_t binData2[]={0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
					   0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01};
	size_t binLen = sizeof(binData);
	
	fileNum = fileOpen("adump.bin","w");
	for(uint16_t a=0;a<2048;a++) fileWrite(binData,  sizeof(uint8_t), binLen, fileNum); //first half
	for(uint16_t a=0;a<2048+16;a++) fileWrite(binData2, sizeof(uint8_t), binLen, fileNum); //second half, and should overflow into first half of the file
	fileClose(fileNum);
	
	printf("hit space to quit");
	hitspace();
	
	return 0;}
