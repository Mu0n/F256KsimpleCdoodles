#include "f256lib.h"
#include "../src/textUI.h"

void instructions()
{
textEnableBackgroundColors(true);
textSetColor(13,1);
textGotoXY(0,0);printf("MOD Player                                ");
textGotoXY(0,1);printf("     Created by Mu0n, May 2026 v0.1       ");
}
void textUI()
{	

textGotoXY(0,INSTR_LINE);
textSetColor(7,6);printf("[ESC]");
textSetColor(15,0);printf(" Quit ");
textSetColor(7,6);printf("[Tab]");
textSetColor(15,0);printf(" Skip ");
textSetColor(7,6);printf("[F3]");
textSetColor(15,0);printf(" Load ");
textSetColor(7,6);printf("[F7]");
textSetColor(15,0);printf(" Info ");
textSetColor(7,6);printf("[Space]");
textSetColor(15,0);printf(" Snoop!");

}
void pauseTextUI()
{	
eraseLine(INSTR_LINE);
textGotoXY(0,INSTR_LINE);
textSetColor(7,6);printf("[Space]");
textSetColor(15,0);printf(" Resume");

}
void eraseLine(uint8_t line)
{	
textGotoXY(0,line);printf("                                                                                ");
}

