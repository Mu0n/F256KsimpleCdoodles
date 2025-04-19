#define F256LIB_IMPLEMENTATION

#include "f256lib.h"
#include "../src/muUtils.h" //contains helper functions I often use
#include "../src/muMidi.h" //contains helper functions I often use
#include "../src/muopl3.h" //contains helper functions I often use
#include "../src/musid.h" //contains helper functions I often use


EMBED(midistrs, "../src/midi.txt", 0x20000);

char assemb[100];
typedef struct midiNameFP
{
	uint8_t len;
	uint16_t offset;
} mNFP;


struct midiNameFP names[128];


uint8_t foundProg(char *);

uint8_t foundProg(char *fileName)
{
	char *dirOpenResult;
	struct fileDirEntS *myDirEntry;
	uint8_t each=0;
	
	printf("%s\nlen %d\n",fileName,strlen(fileName));
	//checking the contents of the directory
	dirOpenResult = fileOpenDir("0:music");
	while((myDirEntry = fileReadDir(dirOpenResult))!= NULL)
	{
		if(_DE_ISREG(myDirEntry->d_type)) 
		{
			each = strcmp(myDirEntry->d_name,fileName);
			if(each==0) {
				
				fileCloseDir(dirOpenResult);
				return 1;
				}
		}
	}
	fileCloseDir(dirOpenResult);
	return 0;
}

void clearOut()
{
	for(uint8_t j=0;j<100;j++) assemb[j] = 0;
}

uint8_t readInst(FILE *fileNum)
{
	char cr=0;
	uint8_t i=0;
	while(true)
	{
	fread(&cr, 1, 1,fileNum);
	if(cr=='\n') break;
	assemb[i++]=cr;
	}
	assemb[i]='\0';
	return i;
}
void openMidiNames()
{
	FILE *fileNum =0;
	uint8_t i=0,count=0;
	uint32_t soFar=0x10000;
	uint8_t j=0;
	uint16_t of=0;
	
	//making a file and writing in it, in the root
	printf("\nAttempting to read midi instrument names");
	fileNum = fileOpen("0:music/midi.txt","r");
	
	while(count<128)
		{
		i=readInst(fileNum);
		for(j=0;j<i;j++)
		{
			FAR_POKE(soFar++,assemb[j]);
		}
		names[count].len = i;
		of +=(uint16_t)i;
		names[count].offset = of; 
		count++;
		//printf("\n%s",assemb);
		}
	fileClose(fileNum);
	kernelNextEvent();
}

void typeInst(uint8_t which)
{
	printf("\nwhich: %d, len %d, offset %02x\n", which, names[which].len, names[which].offset);
	for(uint8_t i=0; i<names[which].len; i++)
	{
		printf("%c",FAR_PEEK(0x10000+(uint32_t)names[which].offset+(uint32_t)i));
	}
}

int main(int argc, char *argv[]) {
	uint8_t findResult=0;
	findResult = foundProg("FireJam.pgz");
	if(findResult==1) printf("\nfound FireJam.pgz in music/");
	else printf("\ndid not find FireJam.pgz in music/");
	hitspace();
	openMidiNames();
	hitspace();
	printf("\n");
	for(uint8_t i=0;i<8;i++) typeInst(i);
	hitspace();
	
	return 0;}
