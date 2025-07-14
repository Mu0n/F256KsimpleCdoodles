#include "D:\F256\llvm-mos\code\overlay2\.builddir\trampoline.h"

#include "f256lib.h"
#include "../src/extra.h"

#define SEGMENT_FIRST
#pragma clang optimize off
__attribute__((noinline))
void doSomething() {
	volatile unsigned char ___mmu = (unsigned char)*(volatile unsigned char *)0x000d;
	*(volatile unsigned char *)0x000d = 8;
	FAR8_doSomething();
	*(volatile unsigned char *)0x000d = ___mmu;
}
#pragma clang optimize on

__attribute__((noinline, section(".block8")))
void FAR8_doSomething() {
	printf("******************************\n");
    printf("*                            *\n");
    printf("*     Welcome to PGZ Box     *\n");
    printf("*                            *\n");

    // Filler text lines
    printf("*  This is a test message.   *\n");
    printf("*  It simulates a text box.  *\n");
    printf("*                            *\n");
    printf("*  You can change the text   *\n");
    printf("*  or make it interactive.   *\n");
    printf("*                            *\n");
    printf("*  Segment loading complete. *\n");
    printf("*  Memory checks passed.     *\n");
    printf("*                            *\n");
    printf("*  Starting execution at     *\n");
    printf("*  address 0x300   .         *\n");
    printf("*                            *\n");
    printf("*  Execution finished.       *\n");
    printf("*  Press any key to exit.    *\n");
    printf("*                            *\n");

    // Bottom border
    printf("******************************\n");}
	
