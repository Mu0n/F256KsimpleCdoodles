#include "C:\F256\f256llvm-mos\F256KsimpleCdoodles\multiRead\.builddir\trampoline.h"

#define F256LIB_IMPLEMENTATION

#include "f256lib.h"
#include "../src/muUtils.h" //contains helper functions I often use

uint16_t readBigEndian16(FILE *fp) {
    uint8_t bytes[2];
    if (fread(bytes, 1, 2, fp) != 2) {
        // Handle error — possibly end of file
        return 0;
    }
    return ((uint16_t)bytes[0] << 8) |
           (uint16_t)bytes[1];
}



uint32_t readBigEndian32(FILE *fp) {
    uint8_t bytes[4];
    if (fread(bytes, 1, 4, fp) != 4) {
        // Handle error — possibly end of file
        return 0;
    }
	printf("%02x %02x %02x %02x\n", bytes[0], bytes[1], bytes[2], bytes[3]);
    return ((uint32_t)bytes[0] << 24) |
           ((uint32_t)bytes[1] << 16) |
           ((uint32_t)bytes[2] << 8)  |
           (uint32_t)bytes[3];
}


int main(int argc, char *argv[]) {
	uint8_t *fileNum =0, *fileNum2 = 0;
	uint8_t cmdBuf[8], cmdBuf2[8];
	uint16_t nbTracks = 0;
	uint32_t pos;
	
	uint32_t *lengths, *offsets, *starts;
	bool *isDone;
	//size_t binLen = sizeof(binData);
	
	//setup and open file
	
	for(uint8_t i=0; i<8; i++) cmdBuf[i] = 0; // clear out
	
	fileNum  = fileOpen("Asayake.mid","r");
	fileNum2 = fileOpen("Asayake.mid","r");
	
	
	//read number of tracks
	fseek(fileNum, 10, SEEK_SET);
	nbTracks = readBigEndian16(fileNum);  // Read number of tracks
	
	lengths = (uint32_t *) malloc(sizeof(uint32_t) * nbTracks);
	offsets = (uint32_t *) malloc(sizeof(uint32_t) * nbTracks);
	starts = (uint32_t *) malloc(sizeof(uint32_t) * nbTracks);
	isDone = (bool *) malloc(sizeof(bool) * nbTracks);
	
	for(uint16_t i = 0; i< nbTracks; i++)
	{
		isDone[i] = false;
		lengths[i] = 0;
		starts[i] = 0;
		offsets[i] = 0;
	}
	
	
	hitspace();
	//go to every track
	pos=14;
	fseek(fileNum, pos, SEEK_SET);  // Skip header chunk
	
	for (uint16_t i = 0; i < nbTracks; i++) {
	
		fread(cmdBuf, 1, 4, fileNum);//read track signature
	
		if (strcmp(cmdBuf, "MTrk") != 0) {
			// Handle error: unexpected chunk
		}
	
		uint32_t length = readBigEndian32(fileNum); // read track byte length
		lengths[i] = length;
		pos+=(uint32_t)8;
		printf("Track %d starts at offset %08lx\n", i, pos);
		starts[i] = pos; //know where to begin
	
		pos+=length;
		fseek(fileNum, pos, SEEK_SET);  // Skip to next chunk
	}
	

	while(true) {
	for(uint16_t i = 0; i < nbTracks; i++)
		{
		if(lengths[i]==0) continue;
		fseek(fileNum, starts[i] + offsets[i], SEEK_SET);
		fileRead(cmdBuf, 1,1, fileNum);
		textGotoXY(0 + offsets[i]*3,15 + i*2); printf("%02x ", cmdBuf[0]);
		offsets[i]++;
		}
	hitspace();
	}
	

	printf("file Num is: %d\n", *fileNum);
	
	
	
	fileClose(fileNum);
	fileClose(fileNum2);
	
	printf("hit space to quit");
	hitspace();
	
	return 0;}
