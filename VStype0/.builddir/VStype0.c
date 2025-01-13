#include "D:\F256\llvm-mos\code\VStype0\.builddir\trampoline.h"

#define F256LIB_IMPLEMENTATION

#include "f256lib.h"

#define VS_SCI_CTRL  0xD700
#define vS_SCI_ADDR  0xD701
#define VS_SCI_DATA  0xD702   //2 bytes
#define VS_FIFO_STAT 0xD704   //2 bytes
#define VS_FIFO_DATA 0xD707


EMBED(human2, "../assets/human2.mid", 0x10000);
uint32_t fileSize = 33876;

int main(int argc, char *argv[]) {
uint16_t i=0, j=0;
uint16_t howManySoFar=0;
uint8_t pass = 0;

asm("sei");
/*
//codec enable all lines
POKE(0xD620, 0x1F);
POKE(0xD621, 0x2A);
POKE(0xD622, 0x01);
while(PEEK(0xD622) & 0x01);
*/



POKEW(VS_FIFO_STAT, 0x8000); //force the buffer to be empty? I think?

printf("%04x bytes at start\n",PEEKW(VS_FIFO_STAT)&0x03FF);
while(i<fileSize)
{
	pass++;
	for(i=j;i<j+0x0800;i++) //loop in chunks of 2048 bytes, supposedly the size of the buffer?
		{
		if(howManySoFar + i > fileSize) POKE(VS_FIFO_DATA, 0x00);
		else POKE(VS_FIFO_DATA, FAR_PEEK((uint32_t)0x10000+(uint32_t)i)); //get the next byte from the embedded midi and shove it in the FIFO
		}
	j+=0x0800;
	howManySoFar+=0x0800;
	printf("%04x bytes after pass %d\n",PEEKW(VS_FIFO_STAT)&0x03FF,pass);
	while((PEEKW(VS_FIFO_STAT)& 0x8000) == 0); //loop in circles while it's not empty yet
}

asm("cli");

while(true)
{

}
return 0;}
