#include "D:\F256\llvm-mos\code\fileWrite2\.builddir\trampoline.h"

#define F256LIB_IMPLEMENTATION

#include "f256lib.h"
#include "../src/muUtils.h" //contains helper functions I often use

int main(int argc, char *argv[]) {
	uint8_t *fileNum =0;
	uint8_t binData[]={0xDE,0xAD,0xBE,0xEF};
	size_t binLen = sizeof(binData);
	
	fileNum = fileOpen("coucou.bin","w");
	fileWrite(binData, sizeof(uint8_t), binLen, fileNum);
	kernelNextEvent();
	fileWrite(binData, sizeof(uint8_t), binLen, fileNum);
	kernelNextEvent();
	fileWrite(binData, sizeof(uint8_t), binLen, fileNum);
	kernelNextEvent();
	fileWrite(binData, sizeof(uint8_t), binLen, fileNum);
	kernelNextEvent();
	fileWrite(binData, sizeof(uint8_t), binLen, fileNum);
	kernelNextEvent();
	fileClose(fileNum);
	
	printf("hit space to quit");
	hitspace();
	
	return 0;}
