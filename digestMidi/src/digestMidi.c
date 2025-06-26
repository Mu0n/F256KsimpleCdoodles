#define F256LIB_IMPLEMENTATION

#include "f256lib.h"
#include "../src/muUtils.h" //contains helper functions I often use
#include "../src/muMidi.h"  //contains basic MIDI functions I often use
#include "../src/muMidiPlay.h"  //contains basic MIDI functions I often use
#include "../src/timer0.h"  //contains basic cpu based timer0 functions I often use

#define MIDI_BASE   0x40000 //gives a nice 07ffkb until the parsed version happens
#define MIDI_PARSED 0x50000 //end of ram is 0x7FFFF, gives a nice 256kb of parsed midi

#define HELP_TEXT   0x30000

#define textColorOrange 0x09
#define textColorWhite  0x0F
#define textColorBlack  0x00
#define textColorYellow 0x0D
#define textColorGray   0x05

#define statusTextX   20
#define statusTextY   3

char midiFileNames[64][32];
bool isDimPresent[64];
uint8_t choice = 0; //selection number for the directory browser. is used for placing the arrow character left of a filename
uint8_t fileCount=0;
bool exitFlag = false;
bool shiftActive = false;
bool helpActive = false;
bigParsed theBigList; //master structure to keep note of all tracks, nb of midi events, for playback
midiRec myRecord; //keeps parsed info about the midi file, tempo, etc, for info display

char *helpScreen[] = {
"                         F1: Help Screen                                        ", 
"                                                                                ",
"This is a utility that can convert a standard midi file in .mid format and      ", 

"create a corresponding .dim file with the same file name prefix, in the same    ",
"directory as the original file. The .dim files are simplified for quick loading.", 
"This program\'s working directory is in your root\'s /midi/ folder.               ", 
"                                                                                ",
"Usage- navigate to the file you want and press one of the following keys:       ",
"                                                                                ",
//"Enter: Loads the .mid, analyzes it, asks for main tempo and saves a .dim        ",
"Enter: Loads the .mid, analyzes it and saves a .dim                             ",
"                                                                                ",
"Shift-Enter: Loads the corresponding .dim file (if it exists) and plays it.     ",
"                                                                                ",
"Space: plays whatever is loaded into memory from previous Enter or shift-enter  ",
"                                                                                ",
"                                                                                "
//"Filenames in white are only present in .mid format                              ",
//"Filenames in yellow have a corresponding .dim format and can be played          "	
	
};

char *queryFudge[] = {
"                     Digesting a MIDI File                                      ", 
" The default fudge factor is 25.1658 acting as a speed factor between MIDI event",
" delays and the Foenix\' timer. Lower means faster, higher means slower          ",
"                                                                                ",
" Use UP and DOWN arrow to tweak the value, then press enter. 0 means default.   ",
" Round value between 0 and 100:                                                 "

};


void directory(void);
void textGUI(void);
void setup(void);
void loadAndPlayDim(void);
void justPlayDim(void);
void modalHelp(char *[], uint16_t);
void eraseModalHelp(uint16_t);
void dealKeyReleased(uint8_t);
void dealKeyPressed(uint8_t);


void directory()
{
	char *dirOpenResult;
	struct fileDirEntS *myDirEntry;	
	uint8_t x = 1;
	uint8_t y = 5;
	uint8_t i = 0;
	
	//checking the contents of the directory
	dirOpenResult = fileOpenDir("midi");
	
	myDirEntry = fileReadDir(dirOpenResult);
	while((myDirEntry = fileReadDir(dirOpenResult))!= NULL)
	{
		if(_DE_ISREG(myDirEntry->d_type)) 
		{
			strcpy(midiFileNames[i],myDirEntry->d_name);
			textGotoXY(x,y);printf("%s", myDirEntry->d_name);
			y++; i++; fileCount++;
			
			if(y==59)
			{
				x+=20;
				y=5;
			}
		}
	}
	fileCloseDir(dirOpenResult);
	
	textGotoXY(0,5+choice);textSetColor(textColorOrange,0x00);printf("%c",0xFA);textSetColor(textColorWhite,0x00);
}
void textGUI()
{
	textGotoXY(0,0);textSetColor(textColorLGreen,textColorGray);textPrint("DigestMIDI v0.2");
	textSetColor(textColorWhite,0);printf(" - converts a standard MIDI file");
	textGotoXY(0,1);textPrint("to a simplified a proper format made for playback,");
	textGotoXY(0,2);textPrint("using timers from the kernel and from type0.");

	
	textGotoXY(0,4);textPrint("Contents of /midi/ directory");
	
	textGotoXY(0,59);
	textSetColor(textColorLGreen,textColorGray);printf("F1");
	textSetColor(textColorWhite,0);printf(" Help ");
}

void setup()
{		//wipeBitmapBackground(0x2F,0x2F,0x2F);
	POKE(MMU_IO_CTRL, 0x00);
	// XXX GAMMA  SPRITE   TILE  | BITMAP  GRAPH  OVRLY  TEXT
	POKE(VKY_MSTR_CTRL_0, 0b00001111); //sprite,graph,overlay,text
	// XXX XXX  FON_SET FON_OVLY | MON_SLP DBL_Y  DBL_X  CLK_70
	POKE(VKY_MSTR_CTRL_1, 0b00010000); //font overlay, double height text, 320x240 at 60 Hz;
	
	for(uint8_t i=0;i<64;i++) isDimPresent[i] = false; //initialize this array
	
	textGUI();
	directory();
}	
void loadAndPlayDim()
{	
	char midFileName[32];
	char prefix[] = "midi/";
	uint8_t loadResult=0;

	initBigList(&theBigList);
	initMidiRecord(&myRecord, MIDI_BASE, MIDI_PARSED);
	
	textGotoXY(statusTextX,statusTextY);textSetColor(textColorLGreen,0x00);
	    
	sprintf(midFileName, "%s%s", prefix, midiFileNames[choice]);
	strncpy(midFileName + strlen(midFileName) - 3, "dim", 3);
	
	textGotoXY(statusTextX,statusTextY);printf("Loading %s         ",midFileName);
	
	loadResult=readDigestFile(midFileName,&myRecord,&theBigList);
	
	if(loadResult == 1)
	{
		textSetColor(textColorLGreen,0);
		textGotoXY(statusTextX,statusTextY);printf("No .dim file yet. Use Enter!          ");
		textSetColor(textColorWhite,0);
		return; //nothing was preloaded in this
	}
	textGotoXY(statusTextX,statusTextY);printf("Playing %s                    ",midFileName); 
	
	justPlayDim();
}

void justPlayDim()
{
	if(theBigList.trackcount == 0 || theBigList.TrackEventList == NULL) 
	{
		textSetColor(textColorLGreen,0);
		textGotoXY(statusTextX,statusTextY);printf("No music loaded in RAM yet!      ");
		textSetColor(textColorWhite,0);
		return; //nothing was preloaded in this
	}
	
	textGotoXY(statusTextX,statusTextY);printf("Playing...                       "); 
	if(theBigList.trackcount == 1) playmiditype0(&myRecord, &theBigList);
	else playmidi(&myRecord, &theBigList);
	
	textGotoXY(statusTextX,statusTextY);printf("Done playing.                   "); 
}
void modalHelp(char *textBuffer[], uint16_t size)
{
	textSetColor(textColorLGreen,textColorGray);
	for(uint8_t i = 0; i< size;i++)
		{		
		textGotoXY(0,i+10);
		printf("%s",textBuffer[i]);
		}
		
		/*
	textSetColor(textColorWhite,textColorGray);
	textGotoXY(13,14+12);printf("white");
	textSetColor(textColorYellow,textColorGray);
	textGotoXY(13,15+12);printf("yellow");
*/
	textSetColor(textColorWhite,0);
}
void eraseModalHelp(uint16_t size)
{
	textSetColor(textColorBlack,0);
	
	for(uint8_t i = 0; i< size;i++)
		{		
		textGotoXY(0,i+10);
		printf("                                                                                ");
		}
		
	textSetColor(textColorWhite,0);
}
uint8_t askFudge()
{
	int input=0;

	modalHelp(queryFudge,sizeof(queryFudge)/sizeof(queryFudge[0]));
	while (true)
		{
		kernelNextEvent();
		if(kernelEventData.type == kernelEvent(key.PRESSED))
			{
			if(kernelEventData.key.raw == 0xB6 && input < 100) input++; //raise value
			if(kernelEventData.key.raw == 0xB7 && input > 0  ) input--; //lower value
			if(kernelEventData.key.raw == 0x94) return (uint8_t)input; //quit
			}
		textGotoXY(40,15);printf("%03d",input);
		}	
	return 0;
}
void dealWithFile()
{	
	uint8_t queryAnswer = 0;
	int16_t indexStart = 0; //keeps note of the byte where the MIDI string 'MThd' is, sometimes they're not at the very start of the file!	
	char midFileName[32];
	char prefix[] = "midi/";

	initMidiRecord(&myRecord, MIDI_BASE, MIDI_PARSED);
	initBigList(&theBigList);
	
	textGotoXY(statusTextX,statusTextY);textSetColor(textColorLGreen,0x00);
	    
	sprintf(midFileName, "%s%s", prefix, midiFileNames[choice]);
	
	textGotoXY(statusTextX,statusTextY);printf("Loading %s...         ",midFileName);
	if(loadSMFile(midFileName, MIDI_BASE))
	{
		textGotoXY(statusTextX,statusTextY);printf("Error opening %s.              ",midiFileNames[choice]);
		return;
	}
	textGotoXY(statusTextX,statusTextY);printf("Loaded MIDI %s.                ",midiFileNames[choice]);

	indexStart = getAndAnalyzeMIDI(&myRecord, &theBigList);

	if(indexStart!=-1) //found a place to start in the loaded file, proceed to play
		{
		textGotoXY(statusTextX,statusTextY);printf("Analyzing the MIDI...         ");
		parse(indexStart,false, &myRecord, &theBigList); //count the events and prep the mem allocation for the big list of parsed midi events
		adjustOffsets(&theBigList);
		
		
		queryAnswer = askFudge();
		eraseModalHelp(sizeof(queryFudge)/sizeof(queryFudge[0]));
		
		if(queryAnswer!=0) myRecord.fudge = (float)queryAnswer; //tweak this to change the tempo of the tune
		parse(indexStart, true, &myRecord, &theBigList); //load up the actual event data in the big list of parsed midi events
		textGotoXY(statusTextX,statusTextY);printf("%s %d tracks            ",midFileName,theBigList.trackcount);

		strncpy(midFileName + strlen(midFileName) - 3, "dim", 3);
		textSetColor(textColorLGreen,0x00);textGotoXY(statusTextX,statusTextY);printf("Writing %s...                 ",midFileName);
		writeDigestFile(midFileName,&myRecord, &theBigList);
		textSetColor(textColorLGreen,0x00);textGotoXY(statusTextX,statusTextY);printf("Wrote %s.                     ",midFileName);
		textSetColor(textColorWhite,0x00);
		}
}
void dealKeyReleased(uint8_t keyRaw)
{
switch(keyRaw)
	{		
		case 0x00: //left shift
		case 0x01: //right shift
			shiftActive = false;
			break;
	}
}
void dealKeyPressed(uint8_t keyRaw){
	switch(keyRaw)
	{
		case 146: // top left backspace, meant as reset
			exitFlag = true;
			break;
		case 148: //enter
			if(shiftActive) loadAndPlayDim();
			else dealWithFile();
			textGUI();
			directory();			
			break;	
		case 0xb6: //up arrow
			if(choice!=0) 
				{
				textGotoXY(0,5+choice);textSetColor(textColorWhite,0x00);printf("%c",32);
				choice--;
				textGotoXY(0,5+choice);textSetColor(textColorOrange,0x00);printf("%c",0xFA);textSetColor(textColorWhite,0x00);
				}
			break;
		case 0xb7: //down arrow
			if(choice<fileCount-1) 
				{
				textGotoXY(0,5+choice);textSetColor(textColorWhite,0x00);printf("%c",32);
				choice++;
				textGotoXY(0,5+choice);textSetColor(textColorOrange,0x00);printf("%c",0xFA);textSetColor(textColorWhite,0x00);
				}
			break;
		case 32: //space
			justPlayDim();
			break;
		case 0x81: //F1
			if(helpActive == false)
				{
					modalHelp(helpScreen,sizeof(helpScreen)/sizeof(helpScreen[0]));
					helpActive = true;
				}
			else 
				{
					eraseModalHelp(sizeof(helpScreen)/sizeof(helpScreen[0]));
					helpActive = false;
					textGUI();
					directory();
				}
			break;
		case 0x00: //left shift
		case 0x01: //right shift
			shiftActive = true;
			break; 
	}
	
}
int main(int argc, char *argv[]) {

	setup(); //codec, timer and  graphic setup


	resetInstruments(false); //resets all channels to piano, for sam2695
	midiShutUp(false); //ends trailing previous notes if any, for sam2695


	while(!exitFlag)
	{
	kernelNextEvent();
	if(kernelEventData.type == kernelEvent(key.PRESSED))
		{
		dealKeyPressed(kernelEventData.key.raw);
		}
	if(kernelEventData.type == kernelEvent(key.RELEASED))
		{
		dealKeyReleased(kernelEventData.key.raw);
		}
	}

	return 0;}
	
	