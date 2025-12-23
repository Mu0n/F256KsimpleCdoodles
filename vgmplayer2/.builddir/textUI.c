#include "D:\F256\llvm-mos\code\vgmplayer2\.builddir\trampoline.h"

#include "f256lib.h"
#include "../src/textUI.h"
#include "../src/muopl3.h"
uint8_t playChan = 0;

void instructions()
{
textEnableBackgroundColors(true);
textSetColor(13,1);
textGotoXY(0,0);printf("OPL3 Snooper                               ");
textGotoXY(0,1);printf("     Created by Mu0n, December 2025 v0.6   ");
}
void textUI()
{

textGotoXY(0,INSTR_LINE);
textSetColor(7,6);printf("[ESC]");
textSetColor(15,0);printf(" Quit ");
textSetColor(7,6);printf("[F3]");
textSetColor(15,0);printf(" Load ");
textSetColor(7,6);printf("[F7]");
textSetColor(15,0);printf(" Info in real time ");
textSetColor(7,6);printf("[Space]");
textSetColor(15,0);printf(" Pause and Snoop!");

}

void eraseLine(uint8_t line)
{
textGotoXY(0,line);printf("                                                                                ");
}
void axes_info(uint8_t starty)
{
textGotoXY(START_X_VIS,starty-1);
textSetColor(13,0);printf("MTV CTV MKS CKS MAD CAD MSR CSR MWV CWV FED FBL PER WSE NSL FOE");

for(uint8_t i=0; i<18; i++)
	{
	textGotoXY(0,starty+i);
	printf("CH%02d",i);
	}
	
}

void wipe_inst()
{
	
		textSetColor(15,0);
	for(uint8_t chan=0; chan<19; chan++)
		{
		textGotoXY(0,START_HEIGHT_VIS+chan-1);
		textPrint("                                                                    ");
		}
}

void show_all_inst()
{
	textSetColor(15,0);
	for(uint8_t chan=0; chan<18; chan++)
		{
			show_inst(chan);
		}
	
	textGotoXY(START_X_VIS+48,START_HEIGHT_VIS);printf(" %02x ",chip_VT_PERC);
	textGotoXY(START_X_VIS+52,START_HEIGHT_VIS);printf(" %02x ",chip_enable);
	textGotoXY(START_X_VIS+56,START_HEIGHT_VIS);printf(" %02x ",chip_NOTESEL);
	textGotoXY(START_X_VIS+60,START_HEIGHT_VIS);printf(" %02x ",chip_OPL3_PAIRS);
}

void show_inst(uint8_t chan)
	{

		textGotoXY(START_X_VIS,   chan+START_HEIGHT_VIS);printf(" %02x ",opl3_instrument_defs[chan].OP2_TVSKF);
		textGotoXY(START_X_VIS+4, chan+START_HEIGHT_VIS);printf(" %02x ",opl3_instrument_defs[chan].OP1_TVSKF);
		textGotoXY(START_X_VIS+8, chan+START_HEIGHT_VIS);printf(" %02x ",opl3_instrument_defs[chan].OP2_KSLVOL);
		textGotoXY(START_X_VIS+12,chan+START_HEIGHT_VIS);printf(" %02x ",opl3_instrument_defs[chan].OP1_KSLVOL);
		textGotoXY(START_X_VIS+16,chan+START_HEIGHT_VIS);printf(" %02x ",opl3_instrument_defs[chan].OP2_AD);
		textGotoXY(START_X_VIS+20,chan+START_HEIGHT_VIS);printf(" %02x ",opl3_instrument_defs[chan].OP1_AD);
		textGotoXY(START_X_VIS+24,chan+START_HEIGHT_VIS);printf(" %02x ",opl3_instrument_defs[chan].OP2_SR);
		textGotoXY(START_X_VIS+28,chan+START_HEIGHT_VIS);printf(" %02x ",opl3_instrument_defs[chan].OP1_SR);
		textGotoXY(START_X_VIS+32,chan+START_HEIGHT_VIS);printf(" %02x ",opl3_instrument_defs[chan].OP2_WAV);
		textGotoXY(START_X_VIS+36,chan+START_HEIGHT_VIS);printf(" %02x ",opl3_instrument_defs[chan].OP1_WAV);
		textGotoXY(START_X_VIS+40,chan+START_HEIGHT_VIS);printf(" %02x ",opl3_instrument_defs[chan].CHAN_FEED);
		textGotoXY(START_X_VIS+44,chan+START_HEIGHT_VIS);printf(" %02x ",opl3_instrument_defs[chan].CHAN_FNUM);
	}
	
