#include "D:\F256\llvm-mos\code\fileWrite\.builddir\trampoline.h"

#define F256LIB_IMPLEMENTATION

#include "f256lib.h"
#include "../src/muUtils.h" //contains helper functions I often use

int main(int argc, char *argv[]) {
	FILE *fileNum =0;
	int16_t writeResult = 0;
	char buf[]="Hello my darling coucous!";
	size_t msgLen = strlen(buf);
	uint8_t binData[]={0xDE,0xAD,0xBE,0xEF};
	size_t binLen = sizeof(binData);
	int8_t dirResult;
	char *dirOpenResult;
	struct fileDirEntS *myDirEntry;
	
	
	//making a file and writing in it, in the root
	printf("Attempting to open file coucou.txt in the root");
	fileNum = fileOpen("coucou.txt","w");
	writeResult = fileWrite(buf, sizeof(char), msgLen, fileNum);
	printf("\nWe could write %d bytes",writeResult);
	fileClose(fileNum);
	
	//making a directory in the root
	dirResult = fileMakeDir("aTestDir");
	if(dirResult ==0) printf("\ndir making result is %d",dirResult);
	
	//making a file and writing in it, in the directory
	printf("\nAttempting to open file coucou.bin in aTestDir/");
	fileNum = fileOpen("0:aTestDir/coucou.bin","w");
	fileWrite(binData, sizeof(uint8_t), binLen, fileNum);
	fileClose(fileNum);
	
	//checking the contents of the directory
	dirOpenResult = fileOpenDir("aTestDir");
	printf("\ndirOpenResult result %s",dirOpenResult);
	
	myDirEntry = fileReadDir(dirOpenResult);
	
	printf("\nListing contents of aTestDir");
	while((myDirEntry = fileReadDir(dirOpenResult))!= NULL)
	{
		printf("\n%s", myDirEntry->d_name);
		if(_DE_ISREG(myDirEntry->d_type)) printf(" is a REG.");
		if(_DE_ISDIR(myDirEntry->d_type)) printf(" is a DIR.");
		if(_DE_ISLBL(myDirEntry->d_type)) printf(" is a LBL.");
		if(_DE_ISLNK(myDirEntry->d_type)) printf(" is a LNK.");
	}
	dirResult = fileCloseDir(dirOpenResult);
	printf("\ndfileCloseDir result %d",dirResult);
	
	hitspace();
	
	return 0;}
