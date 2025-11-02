#include "D:\F256\llvm-mos\code\sidtweak\.builddir\trampoline.h"

#include "f256lib.h"
#include "../src/musid.h"
#include "../src/mudispatch.h"

void printInstrumentHeaders()
{
	uint8_t curLine = 2;
	
	
	textGotoXY(0,0);textSetColor(0x0E,0x00);printf("SID Tweak v0.1");
	
	textSetColor(0x0C,0xFF);
	textGotoXY(1,curLine++); printf("        SID Chip wide settings:");
	
	
	textSetColor(0x00,0xFF);
	textGotoXY(1,curLine++); printf("      Filter cuttoff (12 bits):");
	textGotoXY(1,curLine++); printf("                     Resonance:");
	textGotoXY(1,curLine++); printf("                   Filter Mode:");
	textGotoXY(1,curLine++); printf("                          Mode:");
	textGotoXY(1,curLine++); printf("                        Volume:");
	textGotoXY(1,curLine++); printf("Use Osc3 for modulation? (Y/N): ");
	
	
	curLine++;
	
	textSetColor(0x0C,0xFF);
	textGotoXY(1,curLine++); printf("                Voice settings:");
	
	textSetColor(0x00,0xFF);
	textGotoXY(1,curLine++); printf("                       Control:");
	textGotoXY(1,curLine++); printf("                        Attack:");
	textGotoXY(1,curLine++); printf("                         Decay:");
	textGotoXY(1,curLine++); printf("                       Sustain:");
	textGotoXY(1,curLine++); printf("                       Release:");
	textGotoXY(1,curLine++); printf("         Pulse Width (12 bits):");
	
	
	curLine++;
	textSetColor(0x0C,0xFF);
	textGotoXY(1,curLine++); printf("             Current polyphony:");
}

void updateValues()
{
	uint8_t curLine = 3, xAlign = 33;
	textSetColor(0x0E,0x00);
	textGotoXY(xAlign,curLine++); printf("%02x%01x", gPtr->sidValues->fcfHi, gPtr->sidValues->fcfLo & 0x0F);
	textGotoXY(xAlign,curLine++); printf("%01x", (gPtr->sidValues->frr & 0xF0)>>4);
	textGotoXY(xAlign,curLine++); printf("%01x", (gPtr->sidValues->frr & 0x0F));
	textGotoXY(xAlign,curLine++); printf("%01x", (gPtr->sidValues->maxVolume & 0xF0)>>4);
	textGotoXY(xAlign,curLine++); printf("%01x", (gPtr->sidValues->maxVolume & 0x0F));
	textGotoXY(xAlign,curLine++); printf("%c", 'N');
	
	curLine+=2;
	textGotoXY(xAlign,curLine++); printf("%02x", gPtr->sidValues->ctrl);
	textGotoXY(xAlign,curLine++); printf("%01x", (gPtr->sidValues->ad & 0xF0)>>4);
	textGotoXY(xAlign,curLine++); printf("%01x", (gPtr->sidValues->ad & 0x0F));
	textGotoXY(xAlign,curLine++); printf("%01x", (gPtr->sidValues->sr & 0xF0)>>4);
	textGotoXY(xAlign,curLine++); printf("%01x", (gPtr->sidValues->sr & 0x0F));
	textGotoXY(xAlign,curLine++); printf("%01x%02x", (gPtr->sidValues->pwdHi & 0x0F), gPtr->sidValues->pwdLo);
	
	
	curLine++;
	textSetColor(0x0C,0xFF);
	textGotoXY(xAlign,curLine++); printf("6");
}

