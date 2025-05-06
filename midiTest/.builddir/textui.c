#include "D:\F256\llvm-mos\code\midiTest\.builddir\trampoline.h"

#include "f256lib.h"
#include "../src/presetBeats.h"
#include "../src/textui.h"
#include "../src/muopl3.h"
#include "../src/muMidi.h"
#include "../src/musid.h"
#include "../src/muUtils.h"
#include "../src/globals.h"

#define CHAR_EMPTY_CIRC 179
#define CHAR_FILLED_CIRC 180

//chip activity:
void layoutChipAct()
{
	/*
	textSetColor(textColorGreen,00);
	textGotoXY(60,3);textPrint("MIDI Sam");printf(" %c",CHAR_EMPTY_CIRC);
	textGotoXY(60,4);textPrint("MIDI  VS");printf(" %c",CHAR_EMPTY_CIRC);
	textGotoXY(60,5);textPrint("     SID");printf(" %c",CHAR_EMPTY_CIRC);
	textGotoXY(60,6);textPrint(" T/I PSG");printf(" %c",CHAR_EMPTY_CIRC);
	textGotoXY(60,7);textPrint("  YMF262");printf(" %c",CHAR_EMPTY_CIRC);
	*/
}

void refreshChipAct(uint8_t *status)
{/*
	textGotoXY(69,3); if(status[0]>0) {textSetColor(textColorOrange,0); printf("%c",CHAR_FILLED_CIRC);}
	else {textSetColor(textColorGreen,0); printf("%c",CHAR_EMPTY_CIRC);}
	textGotoXY(69,4); if(status[1]>0) {textSetColor(textColorOrange,0); printf("%c",CHAR_FILLED_CIRC);}
	else {textSetColor(textColorGreen,0); printf("%c",CHAR_EMPTY_CIRC);}
	textGotoXY(69,5); if(status[2]>0) {textSetColor(textColorOrange,0); printf("%c",CHAR_FILLED_CIRC);}
	else {textSetColor(textColorGreen,0); printf("%c",CHAR_EMPTY_CIRC);}
	textGotoXY(69,6); if(status[3]>0) {textSetColor(textColorOrange,0); printf("%c",CHAR_FILLED_CIRC);}
	else {textSetColor(textColorGreen,0); printf("%c",CHAR_EMPTY_CIRC);}
	textGotoXY(69,7); if(status[4]>0) {textSetColor(textColorOrange,0); printf("%c",CHAR_FILLED_CIRC);}
	else {textSetColor(textColorGreen,0); printf("%c",CHAR_EMPTY_CIRC);}
	*/
}


//This is part of the text instructions and interface during regular play mode
void refreshInstrumentText(struct glTh *gT)
{
	/*
	textGotoXY(5,30); textPrint("                                   ");
	textGotoXY(5,31); textPrint("                                   ");
	
	textGotoXY(5,30); (gT->chSelect==0)?textSetColor(textColorOrange,0x00):textSetColor(textColorGreen,0x00);
	printf("%03d %s",gT->prgInst[0],midi_instruments[gT->prgInst[0]]);
	textGotoXY(5,31); (gT->chSelect==1)?textSetColor(textColorOrange,0x00):textSetColor(textColorGreen,0x00);
	printf("%03d %s",gT->prgInst[1],midi_instruments[gT->prgInst[1]]);
	textGotoXY(5,32); (gT->chSelect==9)?textSetColor(textColorOrange,0x00):textSetColor(textColorGreen,0x00);
	textPrint("Percussion");
	
	if(gT->isTwinLinked){
		textGotoXY(5,31);
		textSetColor(textColorOrange,0x00);
		printf("%03d %s",gT->prgInst[1],midi_instruments[gT->prgInst[1]]);
		}
	
	textSetColor(textColorOrange,0x00);
	switch(gT->chSelect)
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
	if(gT->isTwinLinked)
	{
			textGotoXY(0,30);printf("%c",0xFA);
			textGotoXY(0,31);printf("%c",0xFA);
			textGotoXY(0,32);textPrint(" ");
	}
*/
}

//This is part of the text instructions and interface during regular play mode
void channelTextMenu(struct glTh *gT)
{
	textSetColor(textColorGreen,0x00);
	
	textGotoXY(0,26);textPrint("[F1] to pick an instrument from a list");
	textGotoXY(0,27);textPrint("[F3] to change your output channel");
	textGotoXY(1,29);textPrint("CH  Instrument");
	textGotoXY(2,30);textPrint("0: ");
	textGotoXY(2,31);textPrint("1: ");
	textGotoXY(2,32);textPrint("9: ");
	textGotoXY(0,33);textPrint("[X] to twin link channels 0 & 1");
	refreshInstrumentText(gT);
}


void refreshBeatTextChoice(struct glTh *gT)
{
	textSetColor(textColorOrange,0x00);
	textGotoXY(46,29); textPrint("          ");
	textGotoXY(46,29); printf("%s",presetBeatCount_names[gT->selectBeat]);
}

//This is part of the text instructions and interface during regular play mode
void beatsTextMenu(struct glTh *gT)
{
	textSetColor(textColorGreen,0x00);
	updateTempoText(gT->mainTempo);
	
	textGotoXY(45,26);textPrint("[F5] to cycle preset beats");
	textGotoXY(45,27);textPrint("[F7] play/stop toggle the beat");
	refreshBeatTextChoice(gT);
}

//This is the part of the text instructions and interface for selecting the sound output of the keyboard
void chipSelectTextMenu(struct glTh *gT)
{
	textSetColor(textColorGreen,0x00);
	textGotoXY(5,57);textPrint("C to Cycle chip choice: ");
	textSetColor(textColorGreen,0x00);
	textGotoXY(5,58);textPrint("M to toggle MIDI:");
	showChipChoiceText(gT);
}

//This restores the full text instructions and interface during regular play mode
void textTitle(struct glTh *gT)
{
	uint16_t c;
	//Text Title area
	
	textSetColor(textColorRed,0x00);textGotoXY(21,0);for(c=8;c>0;c--) printf("%c",0x15+c);
	textGotoXY(30,0);textSetColor(textColorWhite,0x00);printf("FireJam  v1.2");
	textSetColor(textColorRed,0x00);textGotoXY(44,0);for(c=1;c<9;c++) printf("%c",0x15+c);
	textGotoXY(33,1);textSetColor(textColorWhite,0x00);textPrint("by Mu0n");
    textDefineForegroundColor(0,0xff,0xff,0xff);
	textSetColor(textColorBlue,0x00);
	textGotoXY(0,2); textPrint("Plug in a midi controller in the MIDI IN port and play!");
	channelTextMenu(gT);
	textSetColor(textColorBlue,0x00);

	beatsTextMenu(gT);

	//Instrument selection instructions
	textSetColor(textColorGreen,0x00);
	textGotoXY(0,35);printf("[%c] / [%c] to change the instrument",0xF8,0xFB);
	textGotoXY(0,36);printf("[Shift-%c] / [Shift-%c] to move by 10 - [Alt-%c] / [Alt-%c] go to the ends ",0xF8,0xFB,0xF8,0xFB);

	//Note play through spacebar instructions
	textGotoXY(0,40);textPrint("[Space] to play a short note under the red line cursor");
	textGotoXY(0,41);printf("[%c] / [%c] to move the cursor",0xF9,0xFA);
	textGotoXY(0,42);printf("[Shift-%c] / [Shift-%c] to move an octave - [Alt-%c] / [Alt-%c] go to the ends ",0xF9,0xFA,0xF9,0xFA);
	
	
	//Chip status, midi chip choice status
	chipSelectTextMenu(gT);
	
	//Chip activity
	layoutChipAct();
}


void updateTempoText(uint8_t tempo)
{
	textSetColor(textColorGreen,0x00);
	textGotoXY(45,31);textPrint("Tempo BPM: ");textPrintInt(tempo);textPrint("  ");
	textGotoXY(45,32);textPrint("[: -1 ]: +1 Sh-[: -10  Sh-]: +10");
}

//This swaps the text of the midi chip choice
void showMIDIChoiceText(struct glTh *gT)
{
	uint8_t firstColor = textColorOrange, secondColor= textColorGreen;
	
	if(gT->chipChoice > 0) firstColor = textColorGreen; //won't highlight any midi in orange
	else if(gT->wantVS1053)
	{
		firstColor = textColorGreen; //reverse colors, vs highlighted in orange, sam in green
		secondColor = textColorOrange;
	}
	textSetColor(firstColor,0x00);textGotoXY(29,57);
	if(gT->wantVS1053) textPrint(" MIDI sam2695  ");
	else textPrint("[MIDI sam2695] ");
	textSetColor(secondColor,0x00);textGotoXY(29,58);
	if(gT->wantVS1053) textPrint("[MIDI VS1053b] ");
	else textPrint(" MIDI VS1053b  ");
}


//This swaps the text of the chip choice
void showChipChoiceText(struct glTh *gT)
{
	switch(gT->chipChoice)
	{
		case 0: //midi
			showMIDIChoiceText(gT);
			textGotoXY(45,57);textSetColor(textColorGreen,0x00);textPrint("SID   PSG   OPL3 ");
			break;
		case 1: //SID
			showMIDIChoiceText(gT);
			textGotoXY(44,57);textSetColor(textColorOrange,0x00);textPrint("[SID] ");
			textGotoXY(50,57);textSetColor(textColorGreen,0x00);textPrint(" PSG   OPL3  ");
			break;
		case 2: //PSG
			showMIDIChoiceText(gT);
			textGotoXY(44,57);textSetColor(textColorGreen,0x00);textPrint(" SID  ");
			textGotoXY(50,57);textSetColor(textColorOrange,0x00);textPrint("[PSG] ");
			textGotoXY(56,57);textSetColor(textColorGreen,0x00);textPrint(" OPL3  ");
			break;
		case 3: //OPL3
			showMIDIChoiceText(gT);
			textGotoXY(44,57);textSetColor(textColorGreen,0x00);textPrint(" SID   PSG  ");
			textGotoXY(56,57);textSetColor(textColorOrange,0x00);textPrint("[OPL3]");
			break;
	}
}


//In this Instrument Picking mode called by hitting [F1], display all General MIDI instruments in 3 columns
//and highlight the currently activated one for the selected channel
//SID and OPL3 can also be shown, probably in 1 column at first
void instListShow(struct glTh *gT)
{
	uint8_t i, y=1;
	if(gT->chipChoice==0){
	midiShutAChannel(0, gT->wantVS1053);
	midiShutAChannel(1, gT->wantVS1053);
	midiShutAChannel(9, gT->wantVS1053);
	}
	if(gT->chipChoice==1) shutAllSIDVoices();
	if(gT->chipChoice==3) opl3_quietAll();
	realTextClear();
	
	textSetColor(textColorOrange,0x00);
	if(gT->chipChoice==0) //MIDI
	{
		textGotoXY(0,0);textPrint("Select your instrument for channel");printf(" %d",gT->chSelect);textPrint(". [Arrows] [Enter] [Space] [Back]");
		textSetColor(textColorGray,0x00);
		for(i=0; i<sizeof(midi_instruments)/sizeof(midi_instruments[0]);i++)
		{
			textGotoXY(2,y);printf("%003d %s ",i,midi_instruments[i]);
			i++;
			textGotoXY(27,y);printf("%003d %s ",i,midi_instruments[i]);
			i++;if(i==sizeof(midi_instruments)/sizeof(midi_instruments[0])) break;
			textGotoXY(52,y);printf("%003d %s ",i,midi_instruments[i]);
			y++;
		}
	}
	if(gT->chipChoice==1) //SID
	{
		textGotoXY(0,0);textPrint("Select your instrument for SID. [Arrows] [Enter] [Space] [Back]");
		textSetColor(textColorGray,0x00);
		for(i=0; i<sid_instrumentsSize;i++)
		{
			textGotoXY(2,1+y+i);printf("%003d %s ",i,sid_instruments_names[i]);

		}
	}
	if(gT->chipChoice==3) //OPL3
	{
		textGotoXY(0,0);textPrint("Select your instrument for OPL3. [Arrows] [Enter] [Space] [Back]");
		textSetColor(textColorGray,0x00);
		for(i=0; i<opl3_instrumentsSize;i++)
		{
			textGotoXY(2,1+y+i);printf("%003d %s ",i,opl3_instrument_names[i]);

		}
	}
	highLightInstChoice(true, gT);
}

//This function highlights or de-highlights a choice in Instrument Picking mode
void highLightInstChoice(bool isNew, struct glTh *gT)
{
	uint8_t x, y;
	isNew?textSetColor(textColorOrange,0):textSetColor(textColorGray,0);
	
	if(gT->chipChoice==0) //MIDI
	{
		x= 2 + 25 * (gT->prgInst[gT->chSelect]%3);
		y= 1 + (gT->prgInst[gT->chSelect]/3);
		textGotoXY(x,y);printf("%003d %s",gT->prgInst[gT->chSelect],midi_instruments[gT->prgInst[gT->chSelect]]);
	}
	if(gT->chipChoice==1) //SID
	{
		x= 2;
		y= 2 + gT->sidInstChoice;
		textGotoXY(x,y);printf("%003d %s",gT->sidInstChoice,sid_instruments_names[gT->sidInstChoice]);
	}
	if(gT->chipChoice==3) //OPL3
	{
		x= 2;
		y= 2 + gT->opl3InstChoice;
		textGotoXY(x,y);printf("%003d %s",gT->opl3InstChoice,opl3_instrument_names[gT->opl3InstChoice]);
	}
}

//These four next functions are for moving around your selection in Instrument Picking mode
void modalMoveUp(struct glTh *gT, bool shift)
{
	highLightInstChoice(false,gT);
	if(gT->chipChoice==0)gT->prgInst[gT->chSelect] -= (3 + shift*27);
	if(gT->chipChoice==1)gT->sidInstChoice--;
	if(gT->chipChoice==3)gT->opl3InstChoice--;
	highLightInstChoice(true,gT);
}
void modalMoveDown(struct glTh *gT, bool shift)
{
	highLightInstChoice(false,gT);
	if(gT->chipChoice==0)gT->prgInst[gT->chSelect] += (3 + shift*27);
	if(gT->chipChoice==1)gT->sidInstChoice++;
	if(gT->chipChoice==3)gT->opl3InstChoice++;
	highLightInstChoice(true,gT);
}
void modalMoveLeft(struct glTh *gT)
{
	highLightInstChoice(false,gT);
	if(gT->chipChoice==0)gT->prgInst[gT->chSelect] -= 1;
	highLightInstChoice(true,gT);
}
void modalMoveRight(struct glTh *gT)
{
	highLightInstChoice(false,gT);
	if(gT->chipChoice==0)gT->prgInst[gT->chSelect] += 1;
	highLightInstChoice(true,gT);
}

