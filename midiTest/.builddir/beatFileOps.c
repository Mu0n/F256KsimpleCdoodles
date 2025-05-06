#include "D:\F256\llvm-mos\code\midiTest\.builddir\trampoline.h"

#include "f256lib.h"
#include "../src/presetBeats.h"
#include "../src/muUtils.h"
/*
void restoreBeat(char *name, struct aB *theB, uint8_t whichBeat)
{
	FILE *fileNum;
	uint8_t *temp = NULL;
	uint32_t addr = 0x10000;
	uint8_t tempCount;
	temp = malloc(sizeof(uint8_t));
	
	fileNum = fileOpen(name,"w");
	fread(temp, sizeof(uint8_t), 1, fileNum);
	theB[whichBeat].suggTempo = *temp;  //tempo
	
	fread(temp, sizeof(uint8_t), 1, fileNum);
	theB[whichBeat].howManyChans = *temp;  //how many "tracks"/"channels" used
	
	for(uint8_t i=0; i<theB[whichBeat].howManyChans; i++)
	{
	
	fread(temp, sizeof(uint8_t), 1, fileNum);
	FAR_POKE(addr++,*temp);    //chip
	
	fread(temp, sizeof(uint8_t), 1, fileNum);
	FAR_POKE(addr++,*temp);    //chan
	
	fread(temp, sizeof(uint8_t), 1, fileNum);
	FAR_POKE(addr++,*temp);    //inst
	
	fread(temp, sizeof(uint8_t), 1, fileNum);
	tempCount = *temp;
	FAR_POKE(addr++,*temp);   //count
	
	for(uint8_t j=0; j<tempCount;j++)
		{
		fread(temp, sizeof(uint8_t), 1, fileNum);
		FAR_POKE(addr++,*temp);
		fread(temp, sizeof(uint8_t), 1, fileNum);
		FAR_POKE(addr++,*temp);
		}
	
	}
	fileClose(fileNum);
	free(temp);
}
*/
void backupBeat(char *name, struct aB *theB, uint8_t whichBeat)
{
	FILE *fileNum;
	uint8_t *temp = NULL;
	uint32_t addr;
	uint8_t countTemp;
	
	temp = malloc(sizeof(uint8_t));

	fileNum = fileOpen(name,"w");
		//just once at the start, one per jingle

		*temp = theB[whichBeat].suggTempo;
		fileWrite(temp, sizeof(uint8_t), 1, fileNum);    //tempo
		*temp = theB[whichBeat].howManyChans;
		fileWrite(temp, sizeof(uint8_t), 1, fileNum); //how many "tracks"/"channels" used
		addr = theB[whichBeat].baseAddr;

		//for every track/beat part of the jingle
		for(uint8_t i=0; i<theB[whichBeat].howManyChans; i++)
			{
			*temp = FAR_PEEK(addr++);
			fileWrite(temp, sizeof(uint8_t), 1, fileNum);    //chip
			*temp = FAR_PEEK(addr++);
			fileWrite(temp, sizeof(uint8_t), 1, fileNum);    //chan
			*temp = FAR_PEEK(addr++);
			fileWrite(temp, sizeof(uint8_t), 1, fileNum);    //inst
			countTemp = *temp = FAR_PEEK(addr++);
			fileWrite(temp, sizeof(uint8_t), 1, fileNum);    //count
	
			for(uint8_t j=0; j<countTemp; j++)
				{
				*temp = FAR_PEEK(addr++);
				fileWrite(temp, sizeof(uint8_t), 1, fileNum);    //a track's data
				*temp = FAR_PEEK(addr++);
				fileWrite(temp, sizeof(uint8_t), 1, fileNum);    //double it because we need the delays too
				}
			}
	fileClose(fileNum);
	free(temp);
	
}
