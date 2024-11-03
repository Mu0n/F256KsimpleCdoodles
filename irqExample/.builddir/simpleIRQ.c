#include "D:\F256\llvm-mos\code\irqExample\.builddir\trampoline.h"


#define F256LIB_IMPLEMENTATION
#define SYS_CTRL_TEXT_PG 2
#define SYS_CTRL_1 0x0001


#include "f256lib.h"

void irqHandler()
{
	uint8_t var,keepMMU,theChar;
	
	textGotoXY(0,31);printf("reached irqHandler");
	
	keepMMU = PEEK(MMU_IO_CTRL);
	var = CHECK_BIT(PEEK(INT_PEND_0),INT00_VKY_SOF);

	if(var==1)
	{
		POKE(INT_PEND_0,var); //clear the flag for SOF
		POKE(SYS_CTRL_1,SYS_CTRL_TEXT_PG); //move to text screen page
		theChar = PEEK(0xC000);
		theChar++;
		POKE(0xC000,theChar); //increment the character at c000
	}
	POKE(MMU_IO_CTRL,keepMMU);
//	asm("rti");
}

int main(int argc, char *argv[]) {
	uint8_t lob, hib;

	lob= LOW_BYTE((uint16_t)irqHandler);
	hib=HIGH_BYTE((uint16_t)irqHandler);
	
	POKE(VKY_MSTR_CTRL_0,1); //text overlay
	POKE(VKY_MSTR_CTRL_1,0); //font mode 80x80

	

	textGotoXY(0,2);printf("MMU_IO_CTRL %04x ",MMU_IO_CTRL);
	textGotoXY(0,3);printf("MMU_IO_PAGE_0 %04x ",MMU_IO_PAGE_0);
    textGotoXY(0,4);printf("MMU_MEM_CTRL %04x ",MMU_MEM_CTRL);
    textGotoXY(0,5);printf("SYS_CTRL_1 %04x ",SYS_CTRL_1);
	
	asm("sei");
	POKE(MMU_MEM_CTRL, 0xB3); // edit mlut 2 active mlut 0
	
	POKE(MMU_MEM_BANK_0, 0x00);
	POKE(MMU_MEM_BANK_1, 0x01);
	POKE(MMU_MEM_BANK_2, 0x02);
	POKE(MMU_MEM_BANK_3, 0x03);
	POKE(MMU_MEM_BANK_4, 0x04);
	POKE(MMU_MEM_BANK_5, 0x05);
	POKE(MMU_MEM_BANK_6, 0x06);
	POKE(MMU_MEM_BANK_7, 0x3F); //tweaked last 2k block for my own vector setting purposes
	
	POKE(MMU_MEM_CTRL, 0x03); //turn off mlut editing, switch to MLUT 2

	POKE(VIRQ,lob);
	POKE(VIRQ+1,hib);
	
	
	//setup();
	textGotoXY(0,20);printf("INT_PEND_0 %04x ",INT_PEND_0);
	textGotoXY(0,21);printf("INT_PEND_1 %04x ",INT_PEND_1);
	textGotoXY(0,22);printf("INT_MASK_0 %04x ",INT_MASK_0);
	textGotoXY(0,23);printf("INT_MASK_1 %04x ",INT_MASK_1);
	textGotoXY(0,24);printf("VIRQ is at %04x ",VIRQ);
	
	textGotoXY(0,30);printf("lo_b of irqHandler: %02x ",lob);
	textGotoXY(0,31);printf("hi_b of irqHandler: %02x ",hib);
	textGotoXY(0,32);printf("all together: %04x ",(uint16_t)irqHandler);
	
	textGotoXY(0,40);printf("lo_b of VIRQ: %02x ",PEEK(VIRQ));
	textGotoXY(0,41);printf("hi_b of  VIRQ: %02x ",PEEK(VIRQ+1));
	textGotoXY(0,42);printf("all together: %04x ",PEEKW(VIRQ));
	
	textGotoXY(0,40);printf("lo_b of VIRQ: %02x ",PEEK(VIRQ));
	textGotoXY(0,41);printf("hi_b of  VIRQ: %02x ",PEEK(VIRQ+1));
	textGotoXY(0,42);printf("all together: %04x ",PEEKW(VIRQ));
	POKE(INT_MASK_1,0xFF); //mask all except SOF interrupt
	
	
	textGotoXY(0,43);printf("INT00_VKY_SOF: %04x ",INT00_VKY_SOF);
	textGotoXY(0,44);printf("(~INT00_VKY_SOF): %04x ",(~INT00_VKY_SOF));
	POKE(INT_MASK_0,(0xFF) & (~INT00_VKY_SOF));
	
	POKE(INT_PEND_0,0xFF); //clear both pending interrupts
	POKE(INT_PEND_1,0xFF);
	
	POKE(SYS_CTRL_1,SYS_CTRL_TEXT_PG);
	POKE(0xC000,'@');
	POKE(SYS_CTRL_1,MMU_IO_COLOR);
	POKE(0xC000, 0x0C);
	
	POKE(SYS_CTRL_1, 0);
	POKE(0xD000,0x41); //text overlay with gamma
	POKE(0xD001,0); //font mode 80x80
	asm("cli");
	
	while(true);
	


 
return 0;}
