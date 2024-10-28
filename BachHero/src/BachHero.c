/* This is 1Bit Fever Dreams aka Mu0n aka Michaël Juneau's entry for the Oct 25-27 2024 Game Jam for the F256K/Jr/K2 event

Bach Hero: guitar hero, but using a sam2695 dream IC to hear midi notes as you
attempt to play a musical piece with the notes falling down into a piano
picture, in time. See if your playing gets the approval of Johaness Sebastian Bach!
*/


/* key codes
  50  51      53  54  55      57  48      61
   2   3       5   6   7       9   0       =
113 119 101 114 116 121 117 105 111 112  91  93
 Q   W   E   R   T   Y   U   I   O   P   [   ]
97  115 100     103 104    107 108 59 
 A   S   D       G  H       K   L   ;
  122 120  99 118 98 110 109 44  46  47
   Z   X    C  V   B  N   M   ,   .   /
*/


/* scrolling notes positions

about to hit piano: x=175 y=163
*/
#define F256LIB_IMPLEMENTATION
#include "f256lib.h"

#define MIDI_CTRL 0xDDA0
#define MIDI_OUT 0xDDA1
#define MIDI_RX_00_07 0xDDA2
#define MIDI_RX_08_10 0xDDA3

#define TIMER_FRAMES 0
#define TIMER_SECONDS 1

 //TIMER_TEXT for the 1-frame long text refresh timer for data display
 //TIMER_NOTE is for a midi note timer
#define TIMER_GUI_COOKIE 0
#define TIMER_NOTE_COOKIE 1
#define TIMER_GLASSES_COOKIE 2

#define TIMER_GUI_DELAY 1
#define TIMER_NOTE_DELAY 30
#define INIT_NOTE_DELAY 25
#define TIMER_GLASSES_DELAY 3

#define REF_MIDI_CHAN 0
#define PLY_MIDI_CHAN 1

#define MAX_SONG_INDEX 82
#define SPR_NOTE_DATA  0x30000
#define GLASSES_ADDR_1 0x38000
#define GLASSES_ADDR_2 0x40000
#define CURSOR_END_SONG 73

EMBED(xaa, "../assets/xaa", 0x10000);
EMBED(xab, "../assets/xab", 0x18000);
EMBED(xac, "../assets/xac", 0x20000);
EMBED(palbach, "../assets/bachbm.pal", 0x28000);
EMBED(ntespr, "../assets/note.spr", 0x30000);
EMBED(glasses1, "../assets/glasses1.spr", 0x38000);
EMBED(glasses2, "../assets/glasses2.spr", 0x40000);

struct timer_t midiTimer, GUITimer, glassesTimer; //timer_t structure for setting timer through the kernel

uint16_t note, oldnote, refnote,songrefnote; /*note is the current midi hex note code to send. oldnote keeps the previous one so it can be Note_off'ed away after the timer expires, or a new note is called. refnote is the correct song playing*/
uint8_t score=0;
uint16_t reticX=179, reticY=159;
bool midiPSG = true;

uint8_t MIDINotesToKeyCodes[128] = {0,0,0,0,0,0,0,0,  //0-7
									0,0,0,0,0,0,0,0,  //8-15
									0,0,0,0,0,0,0,0,  //16-23
									0,0,0,0,0,0,0,0,  //24-31
									0,0,0,0,0,0,0,0,  //32-39
									0,0,97,122,115,120,100,99,  //40-47
									118,103,98,104,110,109,107,44,  //48-55
									108,46,59,47,113,50,119,51,  //56-63
									101,114,53,116,54,121,55,117,  //64-71
									105,57,111,48,112,91,61,93,  //72-79
									0,0,0,0,0,0,0,0,  //80-87
									0,0,0,0,0,0,0,0,  //88-95
									0,0,0,0,0,0,0,0,  //96-103
									0,0,0,0,0,0,0,0,  //104-111
									0,0,0,0,0,0,0,0,  //112-119
									0,0,0,0,0,0,0,0}; //120-127
									
uint8_t KeyCodesToMIDINotes[123] = {0,0,0,0,0,0,0,0,  //0-7
									0,0,0,0,0,0,0,0,  //8-15
									0,0,0,0,0,0,0,0,  //16-23
									0,0,0,0,0,0,0,0,  //24-31
									0,0,0,0,0,0,0,0,  //32-39
									0,0,0,0,55,0,57,59,  //40-47
									75,0,61,63,0,66,68,70,  //48-55
									0,73,0,58,0,78,0,0,  //56-63
									0,0,0,0,0,0,0,0,  //64-71
									0,0,0,0,0,0,0,0,  //72-79
									0,0,0,0,0,0,0,0,  //80-87
									0,0,0,77,0,79,0,0,  //88-95
									0,42,50,47,46,64,0,49,  //96-103
									51,72,0,54,56,53,52,74,  //104-111
									76,60,65,44,67,71,48,62,  //112-119
									45,69,43}; //120-122
uint8_t KeyCodesToLoPSG[123] = {0x9F,0x9F,0x9F,0x9F,0x9F,0x9F,0x9F,0x9F,  //0-7
								0x9F,0x9F,0x9F,0x9F,0x9F,0x9F,0x9F,0x9F, //8-15
								0x9F,0x9F,0x9F,0x9F,0x9F,0x9F,0x9F,0x9F,  //16-23
								0x9F,0x9F,0x9F,0x9F,0x9F,0x9F,0x9F,0x9F,  //24-31
								0x9F,0x9F,0x9F,0x9F,0x9F,0x9F,0x9F,0x9F,  //32-39
								0x9F,0x9F,0x9F,0x9F,0x8B,0x9F,0x8C,0x85,  //40-47
								0x84,0x9F,0x84,0x88,0x9F,0x8E,0x8D,0x80,  //48-55
								0x9F,0x8A,0x9F,0x80,0x9F,0x87,0x9F,0x9F,  //56-63
								0x9F,0x9F,0x9F,0x9F,0x9F,0x9F,0x9F,0x9F,  //64-71
								0x9F,0x9F,0x9F,0x9F,0x9F,0x9F,0x9F,0x9F,  //72-79
								0x9F,0x9F,0x9F,0x9F,0x9F,0x9F,0x9F,0x9F,  //80-87
								0x9F,0x9F,0x9F,0x80,0x9F,0x8F,0x9F,0x9F,  //88-95
								0x9F,0x9F,0x8A,0x8A,0x80,0x83,0x9F,0x87,  //96-103
								0x8F,0x86,0x9F,0x8D,0x8B,0x81,0x87,0x8E,  //104-111
								0x8A,0x8C,0x80,0x9F,0x8D,0x82,0x87,0x8D,  //112-119
								0x89,0x8E,0x9F}; //120-122
uint8_t KeyCodesToHiPSG[123] = {0x9F,0x9F,0x9F,0x9F,0x9F,0x9F,0x9F,0x9F,  //0-7
								0x9F,0x9F,0x9F,0x9F,0x9F,0x9F,0x9F,0x9F,  //8-15
								0x9F,0x9F,0x9F,0x9F,0x9F,0x9F,0x9F,0x9F,  //16-23
								0x9F,0x9F,0x9F,0x9F,0x9F,0x9F,0x9F,0x9F,  //24-31
								0x9F,0x9F,0x9F,0x9F,0x9F,0x9F,0x9F,0x9F,  //32-39
								0x9F,0x9F,0x9F,0x9F,0x23,0x9F,0x1F,0x1C,  //40-47
								0x0B,0x9F,0x19,0x16,0x9F,0x12,0x10,0x0F,  //48-55
								0x9F,0x0C,0x9F,0x1E,0x9F,0x09,0x9F,0x9F,  //56-63
								0x9F,0x9F,0x9F,0x9F,0x9F,0x9F,0x9F,0x9F,  //64-71
								0x9F,0x9F,0x9F,0x9F,0x9F,0x9F,0x9F,0x9F,  //72-79
								0x9F,0x9F,0x9F,0x9F,0x9F,0x9F,0x9F,0x9F,  //80-87
								0x9F,0x9F,0x9F,0x0A,0x9F,0x08,0x9F,0x9F,  //88-95
								0x9F,0x9F,0x2F,0x38,0x3C,0x15,0x9F,0x32,  //96-103
								0x2C,0x0D,0x9F,0x25,0x21,0x28,0x2A,0x0B,  //104-111
								0x0A,0x1A,0x14,0x9F,0x11,0x0E,0x35,0x17,  //112-119
								0x3F,0x0F,0x9F}; //120-122

uint16_t NoteXPosition[80] =       { 0,0,0,0,0,0,0,0,  //0-7
								    0,0,0,0,0,0,0,0,  //8-15
									0,0,0,0,0,0,0,0,  //16-23
									0,0,0,0,0,0,0,0,  //24-31
									0,0,0,0,0,0,0,0,  //32-39
									0,0,163,167,171,175,179,183,  //40-47
									191,194,199,204,207,215,218,223,  //48-55
									227,231,236,239,247,250,255,259,  //56-63
									263,271,274,279,283,287,292,295,  //64-71
									303,306,311,315,319,327,330,335,  //72-79
									};
									
uint8_t songNotes[82]={0, 55,67,69,71,74,72,72,76,74,  //0-8
					   74,79,78,79,74,71,67,69,71,  //9-17
					   72,74,76,74,72,71,69,71,67,  //18-24
					   66,67,69,62,66,69,72,71,69,  //25-33
					   71,67,69,71,74,72,72,76,74,  //34-42
					   74,79,78,79,74,71,67,69,71,  //43-51
					   62,74,72,71,69,67,62,67,66,  //52-60
					   67,71,74,79,74,71,67,71,74,  //61-69
					   79,0 , 0, 0, 0, 0, 0, 0, 0  //70-78
};
									
bool setTimer(const struct timer_t *timer)
{
    *(uint8_t*)0xf3 = timer->units;
    *(uint8_t*)0xf4 = timer->absolute;
    *(uint8_t*)0xf5 = timer->cookie;
    kernelCall(Clock.SetTimer);
	return !kernelError;
}

uint8_t getTimerAbsolute(uint8_t units)
{
    *(uint8_t*)0xf3 = units | 0x80;
    return kernelCall(Clock.SetTimer);
}

void setupNewGame()
{
}

void setPlayerInstrument(uint8_t choice)
{
	POKE(MIDI_OUT, 0xC1);
	POKE(MIDI_OUT, choice);
}
void setInstruments()
{
	POKE(MIDI_OUT,123);
	POKE(MIDI_OUT,0);
	POKE(MIDI_OUT,0xFF);
	//prep the MIDI organ instrument 19
	POKE(MIDI_OUT, 0xC0);
	POKE(MIDI_OUT, 19);
	POKE(MIDI_OUT, 0xC1);
	POKE(MIDI_OUT, 19);
}
//sets the text mode, the organ MIDI instrument and the midi timer, sets up the bitmap for Bach+piano gfx
void setup()
{
	uint16_t c;
	textClear();
	textDefineForegroundColor(0,0xff,0xff,0xff);

	setInstruments();
	
	GUITimer.units = TIMER_FRAMES;
	GUITimer.absolute = TIMER_GUI_DELAY;
	GUITimer.cookie = TIMER_GUI_COOKIE;
	
	midiTimer.units = TIMER_FRAMES;
	midiTimer.absolute = TIMER_NOTE_DELAY;
    midiTimer.cookie = TIMER_NOTE_COOKIE;
	
	glassesTimer.units = TIMER_FRAMES;
	glassesTimer.absolute = TIMER_GLASSES_DELAY;
	glassesTimer.cookie = TIMER_GLASSES_COOKIE;
	
	POKE(1,1); //prep to copy over the palette to the CLUT
	for(c=0;c<1023;c++) POKE(0xD000+c, FAR_PEEK(0x28000+c));
	POKE(1,0);
	bitmapSetAddress(2,0x10000);
	bitmapSetActive(2);
	bitmapSetVisible(2,true);
}

void refreshPrints()
{
	textGotoXY(60,4);
	printf("score: %d ",score);
}

void midiNoteOff(uint8_t wantNote, uint8_t chan)
{
	//Send a Note_Off midi command for the previously ongoing note
    POKE(MIDI_OUT, 0x80 | chan);
	POKE(MIDI_OUT, wantNote);
    POKE(MIDI_OUT, 0x4F);
}
void midiNoteOn(uint8_t wantNote, uint8_t chan)
{
	//Send a Note_On midi command on channel 0		
	POKE(MIDI_OUT, 0x90 | chan);
	POKE(MIDI_OUT, wantNote);
	POKE(MIDI_OUT, 0x7F);
	//printf("%d ",wantNote);
	//keep track of that note so we can Note_Off it when needed
}
void commandNote(uint8_t wantNote, uint8_t chan)
{
	midiNoteOff(oldnote, chan);
	midiNoteOn(wantNote, chan);
}

void titleScreenLoop()
{}

void gameLoop()
{}

void endGameLoop()
{}

void prepSprites()
{
	uint8_t i=0;
	//10 note sprites
	for(i=0;i<9;i++)
	{
		spriteDefine(i, SPR_NOTE_DATA, 8, 0, 0); //id, addr, size, clut, layer
	}
	//glasses sprites for winning condition
	spriteDefine(10, GLASSES_ADDR_1, 32, 0, 0);
	spriteDefine(11, GLASSES_ADDR_2, 32, 0, 0);
}
void drawIncomingNotes(uint8_t curNote, bool isRef)
{
	//uint8_t value1; //where the timer is at right now
	//uint8_t value2; //where the timer will end
	//uint16_t finalValue; //used to compute how high the falling note sprite will be drawn
	uint16_t finalX;
	
	finalX = NoteXPosition[curNote];
	
	/*
	value1 = getTimerAbsolute(TIMER_FRAMES);
	value2 = midiTimer.absolute;
	if(value1 > value2) finalValue = ((uint16_t)(value1)+0xFF) - ((uint16_t)value1);
	else finalValue = (uint16_t)value2 - (uint16_t)value1;
	*/
	//finalY=reticY-(4*finalValue);
	
	if(isRef)
	{
	spriteSetVisible(1,true);
	spriteSetPosition(1,finalX, 192);
	}
	else{
	spriteSetVisible(0,true);
	spriteSetPosition(0,finalX, 192);
	}
	/*
	textGotoXY(4,10);
	printf("N: %d X: %d Y: %d     ",curNote,finalX,finalY);
	*/
	/*
	for(i=0;i<3;i++)
	{
	spriteSetVisible(i,true);
	spriteSetPosition(i,finalX, finalY);
	//spriteSetPosition(i,finalX,finalY-8*i);
	
	}*/
}

void writePianoKeyHelp()
{
	textGotoXY(5,0); printf("Game Jam Oct 25 to 27 2024");
	textGotoXY(5,1); printf("Bach's MIDI Hero by Mu0n aka 1Bit Fever Dreams");
	textGotoXY(60,1); printf("F1: Begin Game!");
	textGotoXY(33,16); printf("A S D   G H   K L ;   2 3   5 6 7   9 0   +");
	textGotoXY(34,23); printf("Z X C V B N M , . / Q W E R T Y U I O P [ ]");
	textGotoXY(12,23); printf("J. S.  Bach");
	textGotoXY(60,2); printf("F3: [MIDI]  PSG ");
	textGotoXY(60,3); printf("F5: Hear Demo ");
}
int main(int argc, char *argv[]) {
	uint8_t noteCursor=0;
	bool isSongActive = false; //real time mode
	bool isTutorialAfterFirst = false; // after first automatic note
	bool isTutorial = false; //tutorial mode
	uint8_t midiInstr = 19;
	int8_t octaveShift=0;
	setup();
	POKE(1,0);
	setTimer(&GUITimer);
	
	prepSprites();
	writePianoKeyHelp();
    while(true) 
        {
		kernelNextEvent();
        if(kernelEventData.type == kernelEvent(timer.EXPIRED))
            {
			switch(kernelEventData.timer.cookie)
				{
				case TIMER_GUI_COOKIE:
					refreshPrints();
					GUITimer.absolute = getTimerAbsolute(TIMER_FRAMES) + TIMER_GUI_DELAY;
					setTimer(&GUITimer); 
					break;
				case TIMER_NOTE_COOKIE:
					midiNoteOff(songrefnote, REF_MIDI_CHAN);
					if(isSongActive && noteCursor<CURSOR_END_SONG)
						{
						noteCursor++;
						songrefnote = songNotes[noteCursor];
						midiTimer.absolute = getTimerAbsolute(TIMER_FRAMES) + INIT_NOTE_DELAY;
						
						if(midiPSG==false){
							POKE(0xD608,0x9F);
							POKE(0xD608,0x94);
							POKE(0xD608,KeyCodesToLoPSG[MIDINotesToKeyCodes[songrefnote]]);//"low byte to PSG channel 1 left"
							POKE(0xD608,KeyCodesToHiPSG[MIDINotesToKeyCodes[songrefnote]]);//hi byte to PSG channel 2 left"
						}
						setTimer(&midiTimer);
						if(midiPSG)midiNoteOn(songrefnote, REF_MIDI_CHAN);
						
						drawIncomingNotes(songrefnote,true);
						}
					else if(isSongActive && noteCursor == CURSOR_END_SONG) {
						POKE(0xD608,0x9F);
						isSongActive = false;
					}
					else if(isTutorialAfterFirst==false && isTutorial==true) 
					{
						isTutorialAfterFirst=true;
						noteCursor++;
						songrefnote = songNotes[noteCursor];
						drawIncomingNotes(songrefnote,true);
					}
					break;
				case TIMER_GLASSES_COOKIE:
					
					reticY++;					
					spriteSetPosition(10,reticX,reticY);
					spriteSetPosition(11,reticX+32,reticY);
					if(reticY < (69+60))
					{
						glassesTimer.absolute = getTimerAbsolute(TIMER_FRAMES) + TIMER_GLASSES_DELAY;
						setTimer(&glassesTimer);
					}
					else 
					{
			        textGotoXY(4,24);printf("Congrats! Here's your reward for winning (MIDI only):");
					textGotoXY(4,25);printf("Hidden commands- Left/Right=change octaves; Up/Down: change instruments");	
					}
					break;
				}	
            }				
		else if(kernelEventData.type == kernelEvent(key.PRESSED))
			{
				//the following line can be used to get keyboard codes
				//printf(" %d  ",kernelEventData.key.raw);
				switch(kernelEventData.key.raw)
				{
					case 146: // top left backspace, meant as reset
						setInstruments();
						octaveShift=0;
						midiInstr=19;
						
						POKE(0xD608,0x9F);
											
						break;
					case 0xb6: //up
						if(midiInstr<129 && midiPSG) {
							midiInstr++;
						    setPlayerInstrument(midiInstr);
						}
						break;
					case 0xb7: //down
						if(midiInstr>0 && midiPSG) {
						midiInstr--;
						setPlayerInstrument(midiInstr);
						}
						break;
					case 0xb8: //left
					midiNoteOff(KeyCodesToMIDINotes[kernelEventData.key.raw]+octaveShift*12, PLY_MIDI_CHAN);
						if(octaveShift>-3 && midiPSG)octaveShift--;
						break;
					case 0xb9: //right
					midiNoteOff(KeyCodesToMIDINotes[kernelEventData.key.raw]+octaveShift*12, PLY_MIDI_CHAN);
						if(octaveShift<4 && midiPSG)octaveShift++;
						break;
						/*
					case 32: //space
						score=0;
						noteCursor=0;
						isSongActive = true;
						midiTimer.absolute = getTimerAbsolute(TIMER_FRAMES) + INIT_NOTE_DELAY;
						songrefnote = songNotes[noteCursor];
						setTimer(&midiTimer);
						midiNoteOn(songrefnote, REF_MIDI_CHAN);
						break;
						*/
					case 129: //F1
						score=0;
						noteCursor=1;
						midiTimer.absolute = getTimerAbsolute(TIMER_FRAMES) + TIMER_NOTE_DELAY;
						songrefnote = songNotes[noteCursor];
						setTimer(&midiTimer);
						midiNoteOn(songrefnote, REF_MIDI_CHAN);
						isTutorial=true;
						isTutorialAfterFirst=false;
						
						textGotoXY(25,8);printf(" ");
						textGotoXY(26,7);
						printf("                      ");
						
						spriteSetVisible(10,false);
						spriteSetVisible(11,false);
						
						break;				
					case 131: //F3 Toggle MIDI and PSG
					
							POKE(0xD608,0x9F);
						if(midiPSG){
							textGotoXY(60,2); printf("F3:  MIDI  [PSG]");
							midiPSG = false;
							textGotoXY(33,16); printf("    D   G H   K L ;   2 3   5 6 7   9 0   +");
							textGotoXY(34,23); printf("  X C V B N M , . / Q W E R T Y U I O P [ ]");
						}
						else{
							textGotoXY(60,2); printf("F3: [MIDI]  PSG ");
							midiPSG = true;
							textGotoXY(33,16); printf("A S D   G H   K L ;   2 3   5 6 7   9 0   +");
							textGotoXY(34,23); printf("Z X C V B N M , . / Q W E R T Y U I O P [ ]");
						}
						break;
					case 133: //F5 Demo mode
						score=0;
						noteCursor=0;
						isSongActive = true;
						midiTimer.absolute = getTimerAbsolute(TIMER_FRAMES) + INIT_NOTE_DELAY;
						
						songrefnote = songNotes[noteCursor];
						if(midiPSG==false){
							POKE(0xD608,0x9F);
							POKE(0xD608,0x94);
							POKE(0xD608,KeyCodesToLoPSG[MIDINotesToKeyCodes[songrefnote]]);//"low byte to PSG channel 1 left"
							POKE(0xD608,KeyCodesToHiPSG[MIDINotesToKeyCodes[songrefnote]]);//hi byte to PSG channel 2 left"
						}
						setTimer(&midiTimer);
						if(midiPSG)midiNoteOn(songrefnote, REF_MIDI_CHAN);
						break;
					default:
						if(midiPSG==false){
							POKE(0xD608,0x9F);
							POKE(0xD608,0x94);
							POKE(0xD608,KeyCodesToLoPSG[kernelEventData.key.raw]);//"low byte to PSG channel 1 left"
							POKE(0xD608,KeyCodesToHiPSG[kernelEventData.key.raw]);//hi byte to PSG channel 2 left"
						}
						else midiNoteOn(KeyCodesToMIDINotes[kernelEventData.key.raw]+octaveShift*12, PLY_MIDI_CHAN);
						drawIncomingNotes(KeyCodesToMIDINotes[kernelEventData.key.raw], false);
						
						
						if(KeyCodesToMIDINotes[kernelEventData.key.raw] == songNotes[noteCursor] && isTutorial)
						{
							score++;
							isTutorialAfterFirst = true;
						}	
						else if(isTutorial && score>0)score--;				
						break;
				}

				//printf("%d %d ",KeyCodesToMIDINotes[kernelEventData.key.raw],songNotes[noteCursor]);


			}
		else if(kernelEventData.type == kernelEvent(key.RELEASED))
			{
				
				if(midiPSG==false) POKE(0xD608,0x9F);
				else midiNoteOff(KeyCodesToMIDINotes[kernelEventData.key.raw]+octaveShift*12, PLY_MIDI_CHAN);
				spriteSetVisible(0,false);
				if(isTutorial && isTutorialAfterFirst && noteCursor < (CURSOR_END_SONG+1))
					{
					isTutorialAfterFirst=false;
					noteCursor++;
					songrefnote = songNotes[noteCursor];
					drawIncomingNotes(songrefnote,true);
					}
			    if(noteCursor==(CURSOR_END_SONG+1) && isTutorial)
					{
					//this is the winning aftermath
					isTutorial = false;
					isTutorialAfterFirst = false;
					textGotoXY(25,8); printf("/");
					textGotoXY(26,7);
					if(score>71) 
					{
						printf("Ausgezeichnet!          ");
						reticX=69;
						reticY=80;
						spriteSetVisible(10,true);
						spriteSetPosition(10,reticX,reticY);
						
						spriteSetVisible(11,true);
						spriteSetPosition(11,reticX+32,reticY);
						glassesTimer.absolute = getTimerAbsolute(TIMER_FRAMES) + TIMER_GLASSES_DELAY;
						setTimer(&glassesTimer);
					}
					else printf("Sehr Schlecht! Wieder!");
					}
			}
        }
return 0;}
