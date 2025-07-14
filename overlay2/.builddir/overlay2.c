#include "D:\F256\llvm-mos\code\overlay2\.builddir\trampoline.h"

#define F256LIB_IMPLEMENTATION
#include "f256lib.h"
#include "../src/extra.h"

#define SEGMENT_MAIN
int main(int argc, char *argv[]) {
	(void)argc;
	(void)argv;
	
	doSomething();
	for(;;);
	return 0;}
