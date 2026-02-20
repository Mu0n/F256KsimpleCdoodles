#include "D:\F256\llvm-mos\code\gjmenuoct25\.builddir\trampoline.h"

#include "f256lib.h"
#include "../src/muMenu.h"
#include "../src/muUtils.h"
#include "../src/menuSound.h" //contains helper functions I often use

const char catNames[4][6] = {"Nov25", "Apr25", "Oct24", "Apr24"};
uint8_t chipActive = 0;

struct menuCatList cats[4];

uint8_t currentCat = 0;

void updateSoundStatus(uint8_t *currentCat) //heart symbol for active sound test playback, clear rest
{
	if(*currentCat!=4)return;
	for(uint8_t i=0;i<5;i++)
	{
		textGotoXY(MENUTOPX, MENUTOPY+1+i);printf("    ");
	}
	if(chipActive>0){textGotoXY(MENUTOPX+2, MENUTOPY+1 +chipActive);printf("%c",252);}
}

void goLeftOrRight(bool left, uint8_t *selected, uint8_t *currentCat) //menu navigating left/right for categories
{
	if(*currentCat == 3 && !left) return;
	if(*currentCat == 0 && left) return;
	
	relaunchTimer(1); //make sound and start timer to kill it
	
	if(left) (*currentCat)--;
	else (*currentCat)++;
	
	//display the contents
	*selected  =0;
	textSetColor(15,0);
	textClear();
	displayMenu(MENUTOPX,MENUTOPY, *currentCat);
	
	//highlight first item
	textSetColor(13,1);
	displayOneItem(MENUTOPX, MENUTOPY, 0, *currentCat);
	
}
void goUpOrDown(bool up, uint8_t *selected, uint8_t cat)  //menu navigating up/down for items
{
	if(*selected == (cats[cat].fillIndex-1) && !up) return;
	if(*selected == 0 && up) return;
	
	relaunchTimer(0); //make sound and start timer to kill it
	
	//un-highlight old item
	textSetColor(15,0);
	displayOneItem(MENUTOPX, MENUTOPY, *selected, cat);
	if(up) (*selected)--;
	else (*selected)++;
	
	//highlight new item
	textSetColor(13,1);
	displayOneItem(MENUTOPX, MENUTOPY, *selected, cat);
}






void initItems() //empty out all the strings
{
for(uint8_t k=0;k<4;k++) //iterate all categories
	{
	for(uint8_t i=0; i<MAXITEMS; i++) //iterate all items
		{
		for(uint8_t j = 0; j < MAXNAME; j++) cats[k].items[i].name[j] = 0;for(uint8_t j = 0; j < MAXNAME; j++) cats[k].items[i].name[j] = 0; //iterate all characters of strings
		for(uint8_t j = 0; j < MAXDESC; j++) cats[k].items[i].desc[j] = 0;
		for(uint8_t j = 0; j < MAXFILE; j++) cats[k].items[i].file[j] = 0;
		}
	cats[k].fillIndex = 0;
	}
}


__attribute__((optnone)) //read from the file vcfmenu.txt
int readLine(FILE *fp) {
    int nameind = 0, descind = 0, fileind = 0, secind = 0; //indices for reading line data
    char c;
	uint8_t argCounter = 0; //says which part of the CSV line data we're at 0=display name, 1=file path, 2=section nb
	char tempNum[3]; //plenty to read just a small menu
	uint8_t whichSection=0, fillHere=0;
	uint8_t prob =0;
	
	prob = fileRead(&c, 1, 1, fp);
    while(prob == 1)
		{
		if (c == 0x0a) //line feed control detected
            break;
        if (c == 0x0d) //carriage return detected
            break;
		if (c == 0x2c) //comma detected
		{
			if(argCounter == 0)
				{
				whichSection = atoi(tempNum);
				fillHere = cats[whichSection].fillIndex;
				}
            argCounter++;
			if(argCounter==MAXARGS) break;
	
			prob = fileRead(&c, 1, 1, fp);
			continue;
		}
		switch(argCounter)
			{
			case 0: //menu section number
				if(secind < 3 - 1) tempNum[secind++] = c;
				break;
			case 1: //program name
				if(nameind < MAXNAME - 1) cats[whichSection].items[fillHere].name[nameind++] = c; //only read up to 49 chars, leave room for ending '\0'
				break;
			case 2: //description
				if(descind < MAXDESC - 1) cats[whichSection].items[fillHere].desc[descind++] = c; //only read up to 49 chars, leave room for ending '\0'
				break;
			case 3: //path and file name
				if(fileind < MAXFILE - 1) cats[whichSection].items[fillHere].file[fileind++] = c; //only read up to 49 chars, leave room for ending '\0'
				break;

			}
	
		prob = fileRead(&c, 1, 1, fp);
		}
	if(fillHere < MAXITEMS && prob == 1)
		{
		cats[whichSection].items[fillHere].name[nameind] = '\0';
		cats[whichSection].items[fillHere].desc[descind] = '\0';
		cats[whichSection].items[fillHere].file[fileind] = '\0';
		cats[whichSection].fillIndex++;
		}
	
    return 1;
}




FILE *loadMenuFile(void) //open up the hardcoded text file for menu content
{
	FILE *theVGMfile;
	theVGMfile = fileOpen("gamejams/menu.txt","r"); // open file in read mode
	if(theVGMfile == NULL) {
		return NULL;
		}
	return theVGMfile;
}


void readMenuContent() //high level reading of the file
{
	FILE *theFile = loadMenuFile();
	for(uint8_t i=0; i<30;i++)
	{
	readLine(theFile);
	}
	fclose(theFile);
}
void displayMenu(uint8_t x, uint8_t y, uint8_t cat) //high level display of the menu contents
{
	for(uint8_t i=0; i < cats[cat].fillIndex; i++)
	{
	displayOneItem(x,y,i,cat);
	}
	
	displayCats(x,y, cat);
}


void displayOneItem(uint8_t x, uint8_t y, uint8_t which, uint8_t cat)
{
	textGotoXY(x,y+which);
	printf("%s",cats[cat].items[which].name);
	
	textGotoXY(x+GAPX,y+which);
	printf("%s",cats[cat].items[which].desc);
	
}
/*
void displayOneItem(uint8_t x, uint8_t y, uint8_t which, uint8_t cat)
{
	uint8_t count=0;
	while(true) //name
	{
		char c = FAR_PEEK(((uint32_t)MENU_BASE +      (uint32_t)(PERITEM) * (uint32_t)which)     +      cat * (uint32_t)(PERCAT));
		if(c == '\0') break;
		textGotoXY(x+count,y+which);
		printf("%c",c);
		count++;
		if(count == MAXNAME) break;
	}
	
	count=0;
	while(true) //desc
	{
		char c = FAR_PEEK(((uint32_t)MENU_BASE +   (uint32_t)MAXNAME +   (uint32_t)(PERITEM) * (uint32_t)which)     +      cat * (uint32_t)(PERCAT));
		if(c == '\0') break;
		textGotoXY(x+count,y+which);
		printf("%c",c);
		count++;
		if(count == MAXDESC) break;
	}
}
*/






void displayCats(uint8_t x, uint8_t y, uint8_t cat)
{
	
	for(uint8_t i=0; i<NBCATS; i++)
	{
		if(cat == i) textSetColor(7,6);
		else textSetColor(15,0);
		textGotoXY(x+3+8*i,y+11);
		printf("%s", catNames[i]);
	}
	
	textSetColor(15,0);
}
