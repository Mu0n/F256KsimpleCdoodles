#include "D:\F256\llvm-mos\code\midiTest\.builddir\trampoline.h"

#define F256LIB_IMPLEMENTATION

#define MIDI_CTRL 	   0xDDA0
#define MIDI_FIFO 	   0xDDA1
#define MIDI_RXD 	   0xDDA2
#define MIDI_RXD_COUNT 0xDDA3
#define MIDI_TXD       0xDDA4
#define MIDI_TXD_COUNT 0xDDA5

#define TIMER_FRAMES 0
#define TIMER_SECONDS 1

 //TIMER_TEXT for the 1-frame long text refresh timer for data display
 //TIMER_NOTE is for a midi note timer
#define TIMER_TEXT_COOKIE 0
#define TIMER_SPACENOTE_COOKIE 50 //will be 50-99

#define TIMER_TEXT_DELAY 1
#define TIMER_SPACENOTE_DELAY 1

#define textColorGreen  0x04
#define textColorOrange 0x09
#define textColorRed    0x91
#define textColorBlue   0x07
#define textColorGray   0x05


#include "f256lib.h"


EMBED(palpiano, "../assets/pian.pal", 0x10000);
EMBED(pia1, "../assets/piaa", 0x18000);
EMBED(pia2, "../assets/piab", 0x20000);
EMBED(pia3, "../assets/piac", 0x28000);

struct timer_t spaceNotetimer, refTimer; //spaceNotetimer: used when you hit space, produces a 1s delay before NoteOff comes in
//refTimer: is 1 frame long, used to display updated text when you hit keys on a midi controller

uint16_t note = 0x36, oldnote, oldCursorNote; /*note is the current midi hex note code to send. oldnote keeps the previous one so it can be Note_off'ed away after the timer expires, or a new note is called*/
uint16_t prgInst[10] = {0,0, 0,0,0,0,0,0,0 ,0}; /* program change value, the MIDI instrument number, for chan 0, 1 and 9=percs */

uint8_t chSelect = 0; //restricted to channel 0, 1 or 9 (for percs)
//lowest note on 88-key piano is a A more than 3 octaves below middle CLUT
//midi note number of that lowest note is dec=21

bool noteColors[88]={1,0,1, 1,0,1,0,1, 1,0,1,0,1,0,1, 1,0,1,0,1, 1,0,1,0,1,0,1, 1,0,1,0,1, 1,0,1,0,1,0,1, 1,0,1,0,1, 1,0,1,0,1,0,1, 1,0,1,0,1, 1,0,1,0,1,0,1, 1,0,1,0,1, 1,0,1,0,1,0,1, 1,0,1,0,1, 1,0,1,0,1,0,1, 1};

const char *midi_instruments[] = {
    "Ac. Grand Piano",
    "Bright Ac. Piano",
    "Electric Grand Piano",
    "Honky-tonk Piano",
    "Electric Piano 1",
    "Electric Piano 2",
    "Harpsichord",
    "Clavinet",
    "Celesta",
    "Glockenspiel",
    "Music Box",
    "Vibraphone",
    "Marimba",
    "Xylophone",
    "Tubular Bells",
    "Santur",
    "Drawbar Organ",
    "Percussive Organ",
    "Rock Organ",
    "Church Organ",
    "Reed Organ",
    "Accordion",
    "Harmonica",
    "Tango Accordion",
    "Ac. Guitar (nylon)",
    "Ac. Guitar (steel)",
    "Elec. Guitar (jazz)",
    "Elec. Guitar (clean)",
    "Elec. Guitar (muted)",
    "Overdriven Guitar",
    "Distortion Guitar",
    "Guitar harmonics",
    "Acoustic Bass",
    "Elec. Bass (finger)",
    "Elec. Bass (pick)",
    "Fretless Bass",
    "Slap Bass 1",
    "Slap Bass 2",
    "Synth Bass 1",
    "Synth Bass 2",
    "Violin",
    "Viola",
    "Cello",
    "Contrabass",
    "Tremolo Strings",
    "Pizzicato Strings",
    "Orchestral Harp",
    "Timpani",
    "String Ensemble 1",
    "String Ensemble 2",
    "SynthStrings 1",
    "SynthStrings 2",
    "Choir Aahs",
    "Voice Oohs",
    "Synth Voice",
    "Orchestra Hit",
    "Trumpet",
    "Trombone",
    "Tuba",
    "Muted Trumpet",
    "French Horn",
    "Brass Section",
    "SynthBrass 1",
    "SynthBrass 2",
    "Soprano Sax",
    "Alto Sax",
    "Tenor Sax",
    "Baritone Sax",
    "Oboe",
    "English Horn",
    "Bassoon",
    "Clarinet",
    "Piccolo",
    "Flute",
    "Recorder",
    "Pan Flute",
    "Blown Bottle",
    "Shakuhachi",
    "Whistle",
    "Ocarina",
    "Lead 1 (square)",
    "Lead 2 (sawtooth)",
    "Lead 3 (calliope)",
    "Lead 4 (chiff)",
    "Lead 5 (charang)",
    "Lead 6 (voice)",
    "Lead 7 (fifths)",
    "Lead 8 (bass + lead)",
    "Pad 1 (new age)",
    "Pad 2 (warm)",
    "Pad 3 (polysynth)",
    "Pad 4 (choir)",
    "Pad 5 (bowed)",
    "Pad 6 (metallic)",
    "Pad 7 (halo)",
    "Pad 8 (sweep)",
    "FX 1 (rain)",
    "FX 2 (soundtrack)",
    "FX 3 (crystal)",
    "FX 4 (atmosphere)",
    "FX 5 (brightness)",
    "FX 6 (goblins)",
    "FX 7 (echoes)",
    "FX 8 (sci-fi)",
    "Sitar",
    "Banjo",
    "Shamisen",
    "Koto",
    "Kalimba",
    "Bag pipe",
    "Fiddle",
    "Shanai",
    "Tinkle Bell",
    "Agogo",
    "Steel Drums",
    "Woodblock",
    "Taiko Drum",
    "Melodic Tom",
    "Synth Drum",
    "Reverse Cymbal",
    "Guitar Fret Noise",
    "Breath Noise",
    "Seashore",
    "Bird Tweet",
    "Telephone Ring",
    "Helicopter",
    "Applause",
    "Gunshot"
};

struct midi_uart {
	uint8_t status;
	uint8_t data;
	uint16_t bytes_in_rx;
	uint16_t bytes_in_tx;
} myMIDIsnapshot, *myMIDIptr;

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

void shutUp()
{
	POKE(MIDI_FIFO, 0xFF);
}

void resetInstruments()
{
	POKE(MIDI_FIFO, 0xC0);
	POKE(MIDI_FIFO, 0);
	POKE(MIDI_FIFO, 0xC1);
	POKE(MIDI_FIFO, 0);
	POKE(MIDI_FIFO, 0xC9);
	POKE(MIDI_FIFO, 0);
	prgInst[0]=0;prgInst[1]=0;prgInst[9]=0;
	shutUp();
}

void realTextClear()
{
	uint16_t c;
	POKE(MMU_IO_CTRL,0x02);
	for(c=0;c<4800;c++)
	{
		POKE(0xC000+c,0x20);
	}
	POKE(MMU_IO_CTRL,0x00);
}

void refreshInstrumentText()
{
	textGotoXY(5,30); textPrint("                                                     ");
	textGotoXY(5,31); textPrint("                                                     ");
	
	textGotoXY(5,30); (chSelect==0)?textSetColor(textColorOrange,0x00):textSetColor(textColorGreen,0x00);
	printf("%03d %s",prgInst[0],midi_instruments[prgInst[0]]);
	textGotoXY(5,31); (chSelect==1)?textSetColor(textColorOrange,0x00):textSetColor(textColorGreen,0x00);
	printf("%03d %s",prgInst[1],midi_instruments[prgInst[1]]);
	textGotoXY(5,32); (chSelect==9)?textSetColor(textColorOrange,0x00):textSetColor(textColorGreen,0x00);
	textPrint("Percussion");
	
	textSetColor(textColorOrange,0x00);
	switch(chSelect)
	{
		case 0:
			textGotoXY(0,30);printf("%c",0xFA);
			textGotoXY(0,31);textPrint(" ");
			textGotoXY(0,32);textPrint(" ");
			break;
		case 1:
			textGotoXY(0,30);textPrint(" ");
			textGotoXY(0,31);printf("%c",0xFA);
			textGotoXY(0,32);textPrint(" ");
			break;
		case 9:
			textGotoXY(0,30);textPrint(" ");
			textGotoXY(0,31);textPrint(" ");
			textGotoXY(0,32);printf("%c",0xFA);
			break;
	}
}

void channelTextMenu()
{
	textSetColor(textColorGreen,0x00);
	
	textGotoXY(0,26);textPrint("[F1] to pick an instrument from a list");
	textGotoXY(0,27);textPrint("[F3] to change your output channel");
	textGotoXY(1,29);textPrint("CH  Instrument");
	textGotoXY(2,30);textPrint("0: ");
	textGotoXY(2,31);textPrint("1: ");
	textGotoXY(2,32);textPrint("9: ");
	refreshInstrumentText();
}
void textTitle()
{
	uint16_t c;
	//Text Title area
	textSetColor(textColorRed,0x00);
	textGotoXY(17,0);for(c=8;c>0;c--) printf("%c",0x15+c);
	textGotoXY(26,0);printf("Mu0n's MIDI test v1.0");
	textGotoXY(48,0);for(c=1;c<9;c++) printf("%c",0x15+c);
    textDefineForegroundColor(0,0xff,0xff,0xff);
	textSetColor(textColorBlue,0x00);
	textGotoXY(0,2); textPrint("Plug in a midi controller in the MIDI IN port and play!");
	channelTextMenu();
	textSetColor(textColorBlue,0x00);
	//Instrument selection instructions
	textGotoXY(0,35);printf("[%c] / [%c] to change the instrument",0xF8,0xFB);
	textGotoXY(0,36);printf("[Shift-%c] / [Shift-%c] to move by 10 - [Alt-%c] / [Alt-%c] go to the ends ",0xF8,0xFB,0xF8,0xFB);


	//Note play through spacebar instructions
	textGotoXY(0,40);textPrint("[Space] to play a short note under the red line cursor");
	textGotoXY(0,41);printf("[%c] / [%c] to move the cursor",0xF9,0xFA);
	textGotoXY(0,42);printf("[Shift-%c] / [Shift-%c] to move an octave - [Alt-%c] / [Alt-%c] go to the ends ",0xF9,0xFA,0xF9,0xFA);
}
void setup()
{
	uint16_t c;
	realTextClear();
	
	POKE(MMU_IO_CTRL, 0x00);
	POKE(VKY_MSTR_CTRL_0, 0x4F); //graphics and tile enabled
	POKE(VKY_MSTR_CTRL_1, 0x00); //320x240 at 60 Hz; double height text
	POKE(VKY_LAYER_CTRL_0, 0x00); //bitmap 0 in layer 0, bitmap 1 in layer 1
	
	POKE(MMU_IO_CTRL,1);  //MMU I/O to page 1
	//prep to copy over the palette to the CLUT
	for(c=0;c<1023;c++)
	{
		POKE(VKY_GR_CLUT_0+c, FAR_PEEK(0x10000+c)); //palette for piano
	}
	
	POKE(MMU_IO_CTRL,0); //MMU I/O to page 0
	
	//piano bitmap at layer 0
	bitmapSetActive(0);
	bitmapSetAddress(0,0x18000);
	bitmapSetVisible(0,true);
	bitmapSetCLUT(0);
	
	textTitle();
	refreshInstrumentText();

	//Prep all the kernel timers

	spaceNotetimer.units = TIMER_SECONDS;
	spaceNotetimer.absolute = getTimerAbsolute(TIMER_SECONDS) + TIMER_SPACENOTE_DELAY;
    spaceNotetimer.cookie = TIMER_SPACENOTE_COOKIE;
	
	refTimer.units = TIMER_FRAMES;
	refTimer.absolute = getTimerAbsolute(TIMER_FRAMES) + TIMER_TEXT_DELAY;
	refTimer.cookie = TIMER_TEXT_COOKIE;
	setTimer(&refTimer);
	
	resetInstruments();
	textSetColor(textColorOrange,0x00);
}

void prgChange(uint8_t prg, uint8_t chan)
{
	POKE(MIDI_FIFO, 0xC0 | chan);
	POKE(MIDI_FIFO, prg);
}

void injectChar(uint8_t x, uint8_t y, uint8_t theChar)
{
		POKE(0x0001,0x02); //set io_ctrl to 2 to access text screen
		POKE(0xC000 + 40 * y + x, theChar);
		POKE(0x0001,0x00);  //set it back to default
}

void highLightInstChoice(bool isNew)
{
	uint8_t x, y;
	isNew?textSetColor(textColorOrange,0):textSetColor(textColorGray,0);
	x= 2 + 25 * (prgInst[chSelect]%3);
	y= 1 + (prgInst[chSelect]/3);
	textGotoXY(x,y);printf("%003d %s",prgInst[chSelect],midi_instruments[prgInst[chSelect]]);
}

void modalMoveUp(bool shift)
{
	highLightInstChoice(false);
	prgInst[chSelect] -= (3 + shift*27);
	highLightInstChoice(true);
}

void modalMoveDown(bool shift)
{
	highLightInstChoice(false);
	prgInst[chSelect] += (3 + shift*27);
	highLightInstChoice(true);
}

void modalMoveLeft()
{
	highLightInstChoice(false);
	prgInst[chSelect] -= 1;
	highLightInstChoice(true);
}

void modalMoveRight()
{
	highLightInstChoice(false);
	prgInst[chSelect] += 1;
	highLightInstChoice(true);
}

void instListModal()
{
	bool shiftHit = false;
	uint8_t i, y=1;
	shutUp();
	realTextClear();
	
	textSetColor(textColorOrange,0x00);
	textGotoXY(0,0);textPrint("Select your instrument for channel");printf(" %d",chSelect);textPrint(". Back to Cancel");
	textSetColor(textColorGray,0x00);
	for(i=0; i<128; i++)
	{
		textGotoXY(2,y);printf("%003d %s ",i,midi_instruments[i]);
		i++;
		textGotoXY(27,y);printf("%003d %s ",i,midi_instruments[i]);
		i++;if(i==128)break;
		textGotoXY(52,y);printf("%003d %s ",i,midi_instruments[i]);
		y++;
	}
	
	highLightInstChoice(true);
	while(true)
        {
		kernelNextEvent();
        if(kernelEventData.type == kernelEvent(key.PRESSED))
			{
				switch(kernelEventData.key.raw)
				{
					case 146: // top left backspace, meant as reset
							resetInstruments();
							realTextClear();
							textTitle();
							return;
					case 129: //F1
							realTextClear();
							textTitle();
							prgChange(prgInst[chSelect],chSelect);
							return;
					case 0xb6: //up arrow
						if(prgInst[chSelect] > (shiftHit?29:2)) modalMoveUp(shiftHit);
						break;
					case 0xb7: //down arrow
						if(prgInst[chSelect] < (shiftHit?98:125)) modalMoveDown(shiftHit);
						break;
					case 0xb8: //left arrow
						if(prgInst[chSelect] > 0) modalMoveLeft();
						break;
					case 0xb9: //right arrow
						if(prgInst[chSelect] < 127) modalMoveRight();
						break;
					case 148: //enter
							prgChange(prgInst[chSelect],chSelect);
							realTextClear();
							textTitle();
							return;
					case 1: //shift modifier
						shiftHit = true;
						break;
				}
			}
		if(kernelEventData.type == kernelEvent(key.RELEASED))
		{
			switch(kernelEventData.key.raw)
			{
				case 1: //shift modifier
					shiftHit = false;
					break;
			}
		}
		}
}
int main(int argc, char *argv[]) {
	uint16_t toDo;
	uint8_t x=0,y=2,i;
	uint8_t recByte, detectedNote, detectedColor;
	bool nextIsNote = false; //detect a 0x9? or 0x8? command, the next is a note byte, used for coloring the keyboard note-rects
	bool isHit = false; // true is hit, false is released
	bool altHit = false, shiftHit = false; //keyboard modifiers
	uint8_t spaceNoteCookieOffset =0; //used to pile on cookies for spacebar activated notes


	POKE(1,0);
 
	setup();
	textGotoXY(x,y);
	note=39;
	oldCursorNote=39;
	
	graphicsDefineColor(0, note+0x61,0xFF,0x00,0x00);
	
	while(true)
        {
		if(!(PEEK(MIDI_CTRL) & 0x02)) //rx not empty
			{
				toDo = PEEKW(MIDI_RXD) & 0x0FFF; //discard top 4 bits of MIDI_RXD+1
	
				//erase the region where the last midi bytes received are shown
				textGotoXY(5,4);textPrint("                                                                                ");textGotoXY(5,4);
	
				//deal with the MIDI bytes and exhaust the FIFO buffer
				for(i=0; i<toDo; i++)
				{
					//get and print byte by byte
					recByte=PEEK(MIDI_FIFO);
					textSetColor(textColorOrange,0x00);printf("%02x ",recByte);
	
					if(nextIsNote) //this block triggers if the previous byte was a NoteOn or NoteOff (0x90,0x80) command previously
					{
						//figure out which note on the graphic is going to be highlighted
						detectedNote = recByte-0x14;
	
						//first case is when the last command is a 0x90 'NoteOn' command
						if(isHit) graphicsDefineColor(0, detectedNote,0xFF,0x00,0xFF); //paint it as a hit note
 						//otherwise it's a 0x80 'NoteOff' command
						else {
							detectedColor = noteColors[detectedNote-1]?0xFF:0x00;
							graphicsDefineColor(0, detectedNote,detectedColor,detectedColor,detectedColor); //swap back the original color according to this look up ref table noteColors
						}
						nextIsNote = false; //return to previous state after a note is being dealt with
					}
					if((recByte & 0xF0 )== 0x90) { //we know it's a 'NoteOn', get ready to analyze the note byte, which is next
						nextIsNote = true;
						isHit=true;
						POKE(MIDI_FIFO, ((recByte & 0xF0 ) | chSelect)); //send it to the chosen channel, only the first byte, and reroute
					}
					else if((recByte & 0xF0  )== 0x80) { //we know it's a 'NoteOff', get ready to analyze the note byte, which is next
						nextIsNote = true;
						isHit=false;
						POKE(MIDI_FIFO, ((recByte & 0xF0 ) | chSelect)); //send it to the chosen channel, only the first byte, and reroute
						}
					else POKE(MIDI_FIFO, recByte); //all other bytes are sent normally
						//analysis of the byte is done, send it back for consumption

				}
			}
	
		kernelNextEvent();
        if(kernelEventData.type == kernelEvent(timer.EXPIRED))
            {
			switch(kernelEventData.timer.cookie)
				{
				//all user interface related to text update through a 1 frame timer is managed here
				case TIMER_TEXT_COOKIE:
					refTimer.absolute = getTimerAbsolute(TIMER_FRAMES) + TIMER_TEXT_DELAY;
					setTimer(&refTimer);
					break;
				//all programmatically note playing is expired here
				case TIMER_SPACENOTE_COOKIE ... (TIMER_SPACENOTE_COOKIE+49):
							spaceNoteCookieOffset--;
							if(spaceNoteCookieOffset==0)
							{
							//Send a Note_Off midi command for the previously ongoing note
							POKE(MIDI_FIFO, 0x80 | chSelect);
							POKE(MIDI_FIFO, oldnote);
							POKE(MIDI_FIFO, 0x4F);
							textGotoXY(0,59);printf("%d    ",spaceNoteCookieOffset);
							}


					break;
				}
            }
		else if(kernelEventData.type == kernelEvent(key.PRESSED))
			{
				switch(kernelEventData.key.raw)
				{
					case 146: // top left backspace, meant as reset
						resetInstruments();
						refreshInstrumentText();
						break;
					case 129: //F1
						instListModal();
						break;
					case 131: //F3
						chSelect++;
						if(chSelect == 2) chSelect = 9;
						if(chSelect == 10) chSelect = 0;
						channelTextMenu();
						refreshInstrumentText();
						shutUp();
						prgChange(prgInst[chSelect],chSelect);
						break;
					case 133: //F5
						realTextClear();
						break;
					case 135: //F7
						break;
					case 0xb6: //up arrow
						if(prgInst[chSelect] < 127 - shiftHit*9) prgInst[chSelect] = prgInst[chSelect] + 1 + shiftHit *9;
						if(altHit) prgInst[chSelect] = 127;
						prgChange(prgInst[chSelect],chSelect);
						refreshInstrumentText();
						break;
					case 0xb7: //down arrow
						if(prgInst[chSelect] > 0 + shiftHit*9) prgInst[chSelect] = prgInst[chSelect] - 1 - shiftHit *9;
						if(altHit) prgInst[chSelect] = 0;
						prgChange(prgInst[chSelect],chSelect);
						refreshInstrumentText();
						break;
					case 0xb8: //left arrow
						if(note > (0 + shiftHit*11)) note = note - 1 - shiftHit * 11;
						if(altHit) note = 0;
						graphicsDefineColor(0, oldCursorNote+0x61,0x00,0x00,0x00);
						graphicsDefineColor(0, note+0x61,0xFF,0x00,0x00);
						oldCursorNote = note;
						break;
					case 0xb9: //right arrow
						if(note < 87 - shiftHit*11) note = note + 1 + shiftHit * 11;
						if(altHit) note = 87;
	
						graphicsDefineColor(0, oldCursorNote+0x61,0x00,0x00,0x00);
						graphicsDefineColor(0, note+0x61,0xFF,0x00,0x00);
						oldCursorNote = note;
						break;
					case 32: //space
							//this command will send a programmed note of the current instrument on channel 0, of note value from the global
							//a timer will expire and auto-dial in its NoteOff command

							//Send a Note_Off midi command for the previously ongoing note
	
							POKE(MIDI_FIFO, 0x80 | chSelect);
							POKE(MIDI_FIFO, oldnote);
							POKE(MIDI_FIFO, 0x4F);
	
							//Send a Note_On midi command on channel 0
							POKE(MIDI_FIFO, 0x90 | chSelect);
							POKE(MIDI_FIFO, note+0x15);
							POKE(MIDI_FIFO, 0x7F);
							//keep track of that note so we can Note_Off it when needed
							oldnote = note+0x15; //make it possible to do the proper NoteOff when the timer expires
	
							//Prepare the next note timer
							spaceNoteCookieOffset++;
							spaceNotetimer.cookie = TIMER_SPACENOTE_COOKIE + spaceNoteCookieOffset; //possible pile up of number used to represent the cookie
							spaceNotetimer.absolute = getTimerAbsolute(TIMER_SECONDS) + TIMER_SPACENOTE_DELAY;
							setTimer(&spaceNotetimer);
							textGotoXY(0,59);printf("%d    ",spaceNoteCookieOffset);
						break;
					case 5: //alt modifier
						altHit = true;
						break;
					case 1: //shift modifier
						shiftHit = true;
						break;
				}
				//the following line can be used to get keyboard codes
				//printf("\n %d",kernelEventData.key.raw);
			}
		else if(kernelEventData.type == kernelEvent(key.RELEASED))
		{
			switch(kernelEventData.key.raw)
			{
				case 5:  //alt modifier
					altHit = false;
					break;
				case 1: //shift modifier
					shiftHit = false;
					break;
			}
		}
	
        }
return 0;}
