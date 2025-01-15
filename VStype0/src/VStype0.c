#define F256LIB_IMPLEMENTATION

#include "f256lib.h"

#define VS_SCI_CTRL  0xD700
#define VS_SCI_ADDR  0xD701
#define VS_SCI_DATA  0xD702   //2 bytes
#define VS_FIFO_STAT 0xD704   //2 bytes
#define VS_FIFO_DATA 0xD707


EMBED(human2, "../assets/human2.mid", 0x10000);
uint32_t fileSize = 33876;
const uint16_t plugin[28] = { /* Compressed plugin */
  0x0007, 0x0001, 0x8050, 0x0006, 0x0014, 0x0030, 0x0715, 0xb080, /*    0 */
  0x3400, 0x0007, 0x9255, 0x3d00, 0x0024, 0x0030, 0x0295, 0x6890, /*    8 */
  0x3400, 0x0030, 0x0495, 0x3d00, 0x0024, 0x2908, 0x4d40, 0x0030, /*   10 */
  0x0200, 0x000a, 0x0001, 0x0050
};

//this is a small code to enable the VS1053b's midi mode; present on the Jr2 and K2. It is necessary for the first revs of these 2 machines
void initVS1053MIDI(void) {
    uint8_t n;
    uint16_t addr, val, i=0;

  while (i<sizeof(plugin)/sizeof(plugin[0])) {
    addr = plugin[i++];
    n = plugin[i++];
    if (n & 0x8000) { /* RLE run, replicate n samples */
      n &= 0x7FFF;
      val = plugin[i++];
      while (n--) {
        //WriteVS10xxRegister(addr, val);
        POKE(VS_SCI_ADDR,addr);
        POKEW(VS_SCI_DATA,val);
        POKE(VS_SCI_CTRL,1);
        POKE(VS_SCI_CTRL,0);
		while (PEEK(VS_SCI_CTRL) & 0x80);
      }
    } else {           /* Copy run, copy n samples */
      while (n--) {
        val = plugin[i++];
        //WriteVS10xxRegister(addr, val);
        POKE(VS_SCI_ADDR,addr);
        POKEW(VS_SCI_DATA,val);
        POKE(VS_SCI_CTRL,1);
        POKE(VS_SCI_CTRL,0);
		while (PEEK(VS_SCI_CTRL) & 0x80);
      }
    }
  }
}


int main(int argc, char *argv[]) {
uint16_t i=0, j=0;
uint16_t howManySoFar=0;
uint8_t pass = 0;

asm("sei");

//codec enable all lines
POKE(0xD620, 0x1F);
POKE(0xD621, 0x2A);
POKE(0xD622, 0x01);
while(PEEK(0xD622) & 0x01);

initVS1053MIDI();


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
}