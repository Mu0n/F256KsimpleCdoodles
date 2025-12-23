#define F256LIB_IMPLEMENTATION

#include "f256lib.h"

int main(int argc, char *argv[]) {
	FILE *fileNum =0;
	int16_t writeResult = 0;
	const char buf[]="abc";
	size_t msgLen = strlen(buf);
	
	//making a file and writing in it, in the root
	fileNum = fileOpen("coucou.txt","w");
	writeResult = fileWrite(buf, sizeof(char), msgLen, fileNum);
	fileClose(fileNum);
	
	asm (
	"lda #$42                       \n"
	"sta $F3                        \n"
	"jmp $FF14                      \n"
	);
	
	/*load the next flash block
	put it in kernel_args_run_block_id
	jump the kernel's RunBlock routine to launch superbasic
	*/
	
	
	return 0;}
