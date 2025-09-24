#include "f256lib.h"
#include "../src/muTextUI.h"
#include "../src/mumusicmap.h"
#include "../src/setup.h"
#include "../src/muUtils.h"
#include "../src/mudispatch.h"
#include <string.h>

char nameVersion[] = {" dispatchMIDI  v0.1 by Mu0n, August 2025                                    "};

void directory(uint16_t tlx, uint8_t tly, struct filePick *fP)
{
	char *dirOpenResult;
	struct fileDirEntS *myDirEntry;	
	uint8_t x = tlx;
	uint8_t y = tly;
	
	fP->choice = 0;
	fP->fileCount = 0;
	
	//checking the contents of the directory
	dirOpenResult = fileOpenDir("midi");
	
	myDirEntry = fileReadDir(dirOpenResult);
	while((myDirEntry = fileReadDir(dirOpenResult))!= NULL)
	{
		if(_DE_ISREG(myDirEntry->d_type)) 
		{
			textGotoXY(x,y);printf("%s", myDirEntry->d_name);
			y++; fP->fileCount++;
			/*
			if(y==DIR_NEXT_COL)
			{
				x+=20;
				y=tly;
			}
			*/
		}
	}
	fileCloseDir(dirOpenResult);
	textGotoXY(tlx-1,tly+fP->choice);textSetColor(textColorOrange,0x00);printf("%c",0xFA);textSetColor(textColorWhite,0x00);
}



//displayInfo: SMF info at the top
void displayInfo(struct midiRecord *rec) {
	uint8_t i=0;
	wipeStatus();
	
	textGotoXY(1,1);
	textSetColor(1,0);textPrint("Filename: ");
	textSetColor(0,0);printf("%s",rec->fileName);
	textGotoXY(1,2);
	textSetColor(1,0);textPrint("Type ");textSetColor(0,0);printf(" %d ", rec->format);
	textSetColor(1,0);textPrint("MIDI file with ");
	textSetColor(0,0);printf("%d ",rec->trackcount);
	textSetColor(1,0);(rec->trackcount)>1?textPrint("tracks"):textPrint("track");
	textSetColor(0,0);textGotoXY(1,7);textPrint("CH Instrument   Mute Target");
	for(i=0;i<16;i++)
	{
		textGotoXY(1,8+i);printf("%02d ",i);
	}
	textGotoXY(4,8+9);textSetColor(10,0);textPrint("Percussion");
	
	textGotoXY(0,25);printf(" ->Currently parsing file %s...",rec->fileName);
}


//supeExtraInfo: text controls at the bottom
void superExtraInfo(struct midiRecord *rec) {
	uint16_t temp;
	
	temp=(uint32_t)((rec->totalDuration)/125000);
	temp=(uint32_t)((((double)temp))/((double)(rec->fudge)));
	rec->totalSec = temp;
	//textGotoXY(68,5); printf("%d:%02d",temp/60,temp % 60);
	textSetColor(1,0);textGotoXY(1,MENU_Y);textPrint("[ESC]: ");
	textSetColor(0,0);textPrint("quit");
	textSetColor(1,0);textPrint("    [SPACE]:");
	textSetColor(0,0);textPrint("  pause    ");
	textSetColor(1,0);textPrint("[F1]   ");
	textSetColor(1,0);textPrint("SAM2695");
    textSetColor(0,0);textPrint("   VS1053b");
	textGotoXY(0,25);printf("%s",nameVersion);
	textSetColor(1,0);textGotoXY(62,MENU_Y);textPrint("[F3]:");
	textSetColor(0,0);textPrint("  Load");
	textGotoXY(1,26);textPrint("  [r] toggle repeat when done");
}

void updateMuteDisplay(uint8_t chan)
{
	textGotoXY(4+15,8+chan);textSetColor(0,0);
	if(muteArray[chan])textPrint("Y");
	else textPrint("N");
}
//channel number and MIDI instrument text strings (12 chars max) and mapping
void updateInstrumentDisplay(uint8_t chan, uint8_t pgr) {
	uint8_t i=0;
	textGotoXY(4,8+chan);textSetColor(chan+1,0);
	if(chan==9)
		{
			textPrint("Percussion");
		}
	else for(i=0;i<12;i++)
	{
		if(midi_instruments[pgr][i]=='\0') textPrint(" ");
		else printf("%c",midi_instruments[pgr][i]);
	}
	
	textGotoXY(4+13+5,8+chan);textSetColor(0,0);
	switch(chipXChannel[chan]&0x0F)
	{
		case 0: //SAM2695
			textPrint("MIDI SAM ");
			break;
		case 1: //SID
			textPrint("SID    ");printf("%x",(chipXChannel[chan])>>4);
			break;
		case 2: //PSG
			textPrint("PSG    ");printf("%x",(chipXChannel[chan])>>4);
			break;
			
	}
	presentArray[chan] = 1;
	updateMuteDisplay(chan);
}

void cycleMuteSelection(int8_t upOrDown) //+1 going down the list, -1 going up the list
{
	uint8_t infiniteGuard = 0; //if it can't find present channels, then quit out altogether and do no cycling
	uint8_t oldIndex = presentIndex;
	
	int8_t startLookingHere = presentIndex;
	if(upOrDown == -1)
		{
		startLookingHere--; infiniteGuard++;
		if(startLookingHere<0) startLookingHere=15;
		while(presentArray[startLookingHere] == 0)
			{
			startLookingHere--; infiniteGuard++;
			if(infiniteGuard >16) return;
			if(startLookingHere<0) startLookingHere=15;	
			}
		presentIndex = startLookingHere;	
		}
	else if(upOrDown ==  1)
		{	
		startLookingHere++; infiniteGuard++;
		if(startLookingHere>15) startLookingHere=0;
		while(presentArray[startLookingHere] == 0)
			{
			startLookingHere++; infiniteGuard++;
			if(infiniteGuard >16) return;
			if(startLookingHere>15) startLookingHere=0;	
			}
		presentIndex = startLookingHere;
		}
	textGotoXY(4+14,8+oldIndex);printf(" ");
	textGotoXY(4+16,8+oldIndex);printf(" ");

	
	textSetColor(textColorOrange,0x00);
	textGotoXY(4+14,8+presentIndex);printf("%c",0xFA);
	textGotoXY(4+16,8+presentIndex);printf("%c",0xF9);
	textSetColor(0,0);	
}
}