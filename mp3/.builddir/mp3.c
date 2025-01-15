#include "D:\F256\llvm-mos\code\mp3\.builddir\trampoline.h"

#define F256LIB_IMPLEMENTATION


#define VS_SCI_CTRL   0xD700
#define    CTRL_Start   0x01
#define    CTRL_RWn     0x02
#define    CTRL_Busy    0x08

#define VS_SCI_ADDR  0xD701
#define VS_SCI_DATA  0xD702   //2 bytes
#define VS_FIFO_STAT 0xD704   //2 bytes
#define VS_FIFO_DATA 0xD707



#include "f256lib.h"

FILE *theMP3file;


uint8_t loadMP3File(char *);
uint32_t totalsize = 10295603;

uint8_t loadMP3File(char *name)
{
	uint32_t fileSize=0;
	char buffer[1024];
	size_t bytesRead;

	printf("Opening file: %s\n",name);
	theMP3file = fopen(name,"rb"); // open file in read mode
	if(theMP3file == NULL)
	{
		printf("Couldn't open the file: %s\n",name);
		return 1;
	}
	return 0;
}

int main(int argc, char *argv[]) {
	
	
uint16_t i=0, j=0;
uint16_t howManySoFar=0;
uint8_t pass = 0;
uint16_t bytesRead;

char buffer[2048];


			//codec enable all lines
	POKE(0xD620, 0x1F);
	POKE(0xD621, 0x2A);
	POKE(0xD622, 0x01);
	while(PEEK(0xD622) & 0x01);

	loadMP3File("ronald.mp3");
	
	
while(i<totalsize)
{
	pass++;
	
	bytesRead = fread(buffer, 1, 2048, theMP3file);
	for(i=j;i<j+0x0800;i++) //loop in chunks of 2048 bytes, supposedly the size of the buffer?
		{
		if(howManySoFar + i > totalsize) POKE(VS_FIFO_DATA, 0x00);
		else POKE(VS_FIFO_DATA, buffer[i-j]); //get the next byte from the embedded midi and shove it in the FIFO
		}
	j+=0x0800;
	howManySoFar+=0x0800;
	printf("%04x bytes after pass %d\n",PEEKW(VS_FIFO_STAT)&0x03FF,pass);
	while((PEEKW(VS_FIFO_STAT)& 0x8000) == 0); //loop in circles while it's not empty yet
}
	
return 0;}

