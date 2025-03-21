#include "D:\F256\llvm-mos\code\VSType0\.builddir\trampoline.h"

#define F256LIB_IMPLEMENTATION

#include "f256lib.h"
#include "../src/muMidi.h"
#include "../src/muUtils.h"

EMBED(human2, "../assets/human2.mid", 0x10000);
uint32_t fileSize = 33876;

int main(int argc, char *argv[]) {
uint16_t i=0, j=0;
uint16_t howManySoFar=0;
uint8_t pass = 0;

//openAllCODEC(); //this should no longer be necessary after kernel update of Feb 19, 2025
boostVSClock();
initVS1053MIDI(); //apply the plugin to enable rtmidi

//check the first 4 bytes and display it in the top corner
for(i=0;i<4;i++)
	printf("%02x",FAR_PEEK((uint32_t)0x10000+(uint32_t)i));

//POKEW(VS_FIFO_STAT, 0x8000); //force the buffer to be empty? I think?

printf("\n%04x bytes at start\n",PEEKW(VS_FIFO_STAT)&0x03FF);

i=0; //reset the index
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


while(true)
{

}
return 0;}
