#include "D:\F256\llvm-mos\code\struct\.builddir\trampoline.h"

#define F256LIB_IMPLEMENTATION

#include "f256lib.h"
#include "string.h"

typedef struct aMIDIEvent {
    uint32_t deltaToGo;
    uint8_t bytecount;
    uint8_t msgToSend[3]; //some events are 2 bytes, some are 3, no biggie to use worst-case scenario
    } aME, *aMEPtr, **aMEH;
 
typedef struct aTableOfEvent {
	uint8_t trackno;
	uint16_t eventcount; //keeps track of total events for playback
	uint16_t eventleft; //rises when building up, lowers when playing back
	aMEPtr trackEvents;
	} aTOE, *aTOEPtr, **aTOEH;
	
typedef struct bigParsedEventList {
	bool hasBeenUsed;
	uint16_t trackcount;
	aTOEPtr TrackEventList;
	} bigParsed, *bigParsedPtr, **bigParsedH;
	
	
void displayStuff(bigParsedPtr bp, char *msg)
{
	printf("%s",msg);
	printf("%d\n", bp->hasBeenUsed);
	printf("%d\n", bp->trackcount);
	printf("%08x\n", (uint16_t)bp->TrackEventList);
}
	
int main(int argc, char *argv[]) {
	bigParsed allo;
	uint8_t i, j;
	
	allo.hasBeenUsed = false;
	allo.trackcount = 0;
	allo.TrackEventList = NULL;
	
	displayStuff(&allo,"before allocating\n");
	
	
	
	allo.trackcount = 3;
	allo.TrackEventList = (aTOEPtr)malloc(sizeof(aTOE) * allo.trackcount );
	displayStuff(&allo,"after allocating\n");
	
	
	printf("\n");
	
	for(i=0;i<allo.trackcount;i++)
	{
		allo.TrackEventList[i].trackno = i;
		allo.TrackEventList[i].eventcount =10;
		allo.TrackEventList[i].eventleft=i+1;
		allo.TrackEventList[i].trackEvents=(aMEPtr)malloc(sizeof(aME) * allo.TrackEventList[i].eventcount);

	
	printf("trackno: %d, eventcount: %d, eventleft: %d, baseAddr: %d\n",allo.TrackEventList[i].trackno,
																				  allo.TrackEventList[i].eventcount,
																				  allo.TrackEventList[i].eventleft,
																				  (uint16_t)allo.TrackEventList[i].trackEvents);
	
		for(j=0; j<i+1; j++)
		{
			allo.TrackEventList[i].trackEvents[j].deltaToGo = (i+1)*(j+2);
			allo.TrackEventList[i].trackEvents[j].bytecount = (i+10)*(j+5);
			printf("  delta: %lu bytecount: %d\n", allo.TrackEventList[i].trackEvents[j].deltaToGo, allo.TrackEventList[i].trackEvents[j].bytecount);
		}
	}
	while(true);
	return 0;}
