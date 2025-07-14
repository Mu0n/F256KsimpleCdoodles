#include "f256lib.h"
#include "../src/muTextUI.h"
#include <string.h>

char nameVersion[] = {"  F256Amp  v2.0 by Mu0n, July 2025                                    "};


void modalHelp(char *textBuffer[], uint16_t size)
{
	textSetColor(3,textColorGray);
	for(uint8_t i = 0; i< size;i++)
		{		
		textGotoXY(0,i+2);
		printf("%s",textBuffer[i]);
		}
}
void eraseModalHelp(uint16_t size){
	textSetColor(textColorBlack,0);
	
	for(uint8_t i = 0; i< size;i++)
		{		
		textGotoXY(0,i+2);
		printf("                                                                                ");
		}
		
	textSetColor(textColorWhite,0);
}
}