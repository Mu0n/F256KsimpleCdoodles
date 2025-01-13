#define F256LIB_IMPLEMENTATION

#define MIDI_CTRL 	   0xDDA0
#define MIDI_FIFO 	   0xDDA1
#define MIDI_RXD 	   0xDDA2
#define MIDI_RXD_COUNT 0xDDA3
#define MIDI_TXD       0xDDA4
#define MIDI_TXD_COUNT 0xDDA5

//VS1053b midi
#define MIDI_CTRL_ALT 	    0xDDB0
#define MIDI_FIFO_ALT 	    0xDDB1
#define MIDI_RXD_ALT	    0xDDB2
#define MIDI_RXD_COUNT_ALT  0xDDB3
#define MIDI_TXD_ALT     	0xDDB4
#define MIDI_TXD_COUNT_ALT  0xDDB5


#include "f256lib.h"

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
        POKE(0xD701,addr);
        POKEW(0xD702,val);
        POKE(0xD700,1);
        POKE(0xD700,0);
		while (PEEK(0xd700) & 0x80);
      }
    } else {           /* Copy run, copy n samples */
      while (n--) {
        val = plugin[i++];
        //WriteVS10xxRegister(addr, val);
        POKE(0xD701,addr);
        POKEW(0xD702,val);
        POKE(0xD700,1);
        POKE(0xD700,0);
		while (PEEK(0xd700) & 0x80);
      }
    }
  }
}

int main(int argc, char *argv[]) {
	uint16_t howMany;
	uint8_t i, detected;
	
	
		//codec enable all lines
	POKE(0xD620, 0x1F);
	POKE(0xD621, 0x2A);
	POKE(0xD622, 0x01);
	while(PEEK(0xD622) & 0x01);
	
	initVS1053MIDI();

	POKE(MIDI_FIFO_ALT, 0x90);
	POKE(MIDI_FIFO_ALT, 0x39);
	POKE(MIDI_FIFO_ALT, 0x4F);
	
	while(true) 
        {
		if(!(PEEK(MIDI_CTRL) & 0x02)) //rx not empty
			{
				howMany= PEEKW(MIDI_RXD) & 0x0FFF; 
				for(i=0; i<howMany; i++)
				{
					detected = (uint8_t)PEEK(MIDI_FIFO);
					POKE(MIDI_FIFO_ALT ,detected);
				}
			}
		}
	
	return 0;}
}