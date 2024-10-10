#include "D:\F256\llvm-mos\code\openfile\.builddir\trampoline.h"

#define F256LIB_IMPLEMENTATION

#include "f256lib.h"
#include <stdlib.h>
#define SCREENCORNER 0xC000


int main(int argc, char *argv[]) {
	
	FILE *theMIDIfile;
	char chunk[255];
	uint8_t offset=0;
	uint8_t lineOffset=1;
	
	theMIDIfile = fopen("human2.mid","r"); // open file in read mode

	if(theMIDIfile != NULL) {
		textPrint("Was able to open the file.\n");
  // Read the content and print it
	
		fread(chunk, 255, 1, theMIDIfile);
		for(offset = 0; offset < 255; offset++)
			{
			printf("%02x ",chunk[offset]);
			if((offset+1)%16 == 0) {
				printf("\n");
				}
			}
		POKE(0x0001,2); //access screen memory in bank 6
		for(offset = 0; offset < 255; offset++)
			{
			if((offset > 0) && (offset%16 == 0)) {
				lineOffset++;
				}
	
			POKE(SCREENCORNER + 50 + offset%16 + lineOffset * 80, chunk[offset]);
			}
		POKE(0x0001,1); //restore io
		fclose(theMIDIfile);
	}
	else textPrint("Can't open the file.");
	
	while(true);
	
	return 0;}
