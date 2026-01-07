#include "f256lib.h"
#include "../src/muTextUI.h"
#include "../src/muFilePicker.h" 
#include "../src/setup.h"
#include <string.h>

char nameVersion[] = {" CozyMIDI  v2.5 by Mu0n, January 2026                                    "};

void directory(uint16_t tlx, uint8_t tly, struct filePick *fP)
{
	char *dirOpenResult;
	struct fileDirEntS *myDirEntry;	
	uint8_t x = tlx;
	uint8_t y = tly;
	
	fP->choice = 0;
	fP->fileCount = 0;
	
	//checking the contents of the directory
	dirOpenResult = fileOpenDir("media/midi");
	
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


void modalHelp(char *textBuffer[], uint16_t size)
{
	textSetColor(textColorLightBlue,textColorGray);
	for(uint8_t i = 0; i< size;i++)
		{		
		textGotoXY(0,i+2);
		printf("%s",textBuffer[i]);
		}
}
void eraseModalHelp(uint16_t size)
{
	textSetColor(textColorBlack,0);
	
	for(uint8_t i = 0; i< size;i++)
		{		
		textGotoXY(0,i+2);
		printf("                                                                                ");
		}
		
	textSetColor(textColorWhite,0);
}
void displayInfo(struct midiRecord *rec) {
	uint8_t i=0;
	wipeStatus();
	
	textGotoXY(1,1);
	textSetColor(1,0);textPrint("Filename: ");
	textSetColor(0,0);printf("%s",name);
	textGotoXY(1,2);
	textSetColor(1,0);textPrint("Type ");textSetColor(0,0);printf(" %d ", rec->format);
	textSetColor(1,0);textPrint("MIDI file with ");
	textSetColor(0,0);printf("%d ",rec->trackcount);
	textSetColor(1,0);(rec->trackcount)>1?textPrint("tracks"):textPrint("track");
	textSetColor(0,0);textGotoXY(1,7);textPrint("CH   Instrument");
	for(i=0;i<16;i++)
	{
		textGotoXY(1,8+i);printf("%02d ",i);
	}
	textGotoXY(INST_NAME_X,8+9);textSetColor(10,0);textPrint("Percussion");
	
	textGotoXY(0,25);printf(" ->Currently parsing file %s...",rec->fileName);
}

void extraInfo(struct midiRecord *rec,struct bigParsedEventList *list) {
	wipeStatus();
	textGotoXY(1,3); 
	textSetColor(0,0);printf("%lu ", getTotalLeft(list));
	textSetColor(1,0);textPrint("total event count");
	textGotoXY(40,2); 
	textSetColor(1,0);textPrint("Tempo: ");
	textSetColor(0,0);printf("%d ",rec->bpm);
	textSetColor(1,0);textPrint("bpm");
	textGotoXY(40,3);
	textSetColor(1,0);textPrint("Time Signature ");
	textSetColor(0,0);printf("%d:%d",rec->nn,1<<(rec->dd));
	textGotoXY(0,25);printf("  ->Preparing for playback...                   ");
}
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
void updateInstrumentDisplay(uint8_t chan, uint8_t pgr) {
	uint8_t i=0,j=0;
	textGotoXY(INST_NAME_X,8+chan);textSetColor(chan+1,0);
	if(chan==9)
		{
			textPrint("Percussion");
			return;
		}
	for(i=0;i<36;i++)
	{

		if(midi_instruments[pgr][i]=='\0') 
		{
			for(j=i;j<36;j++) __putchar(32);
			break;
		}
		__putchar(midi_instruments[pgr][i]);
	}
}
}