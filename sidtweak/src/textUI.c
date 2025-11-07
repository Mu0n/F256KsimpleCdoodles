#include "f256lib.h"
#include "../src/musid.h"
#include "../src/mudispatch.h"
#include "../src/textUI.h"

struct SidField sid_fields[16];
uint8_t indexUI = 0;

uint8_t navWSJumps[16] = {0x33,0x42,0x51,0x31,
						  0x11,0x11,0x11,0x11,
						  0x11,0x11,0x11,0x11,
						  0x11,0x13,0x22,0x31
						  }; //high nibble how to jump back, low nibble how to jump forward while navigating with W and S
void init_sid_field(struct SidField *f, uint8_t *target, bool highNib, uint8_t x, uint8_t y, uint8_t startVal) {
    f->value = startVal;
    f->sidIPtr = target;
    f->isHighNib = highNib;
    f->is_dirty = true;
    f->tX = x;
    f->tY = y;
}

void randomInst()
{
	
sid_fields[0].value=  (uint8_t)randomRead()&0x07;//freq cutoff high
sid_fields[1].value=  (uint8_t)randomRead()&0x0F;//freq cutoff high
sid_fields[2].value=  (uint8_t)randomRead()&0x0F;//freq cutoff low 
sid_fields[3].value=  (uint8_t)randomRead()&0x0F;//resonance
sid_fields[4].value=  (uint8_t)randomRead()&0x0F;//filter mode
sid_fields[5].value=  (uint8_t)randomRead()&0x0F;//O3 off and band mode
sid_fields[7].value=  1<<((uint8_t)randomRead()&0x03);//control waveform
sid_fields[8].value=  (uint8_t)randomRead()&0x0F;//Ring sync gate
sid_fields[9].value=  (uint8_t)randomRead()&0x0F;//Attack
sid_fields[10].value= (uint8_t)randomRead()&0x0F;//Decay
sid_fields[11].value= (uint8_t)randomRead()&0x0F;//Sustain
sid_fields[12].value= (uint8_t)randomRead()&0x0F;//Release
sid_fields[13].value= (uint8_t)randomRead()&0x0F;//Pulse width high
sid_fields[14].value= (uint8_t)randomRead()&0x0F;//Pulse width low
sid_fields[15].value= (uint8_t)randomRead()&0x0F;//Pulse width low

for(uint8_t i=0; i<16; i++)sid_fields[i].is_dirty=true;
updateValues();
	
}
void initSIDFields()
{
	init_sid_field(&(sid_fields[0]),&(gPtr->sidValues->fcfHi),       true, 33, 3, 0x00);//freq cutoff high
	init_sid_field(&(sid_fields[1]),&(gPtr->sidValues->fcfHi),      false, 34, 3, 0x00);//freq cutoff high
	init_sid_field(&(sid_fields[2]),&(gPtr->sidValues->fcfLo),      false, 35, 3, 0x00);//freq cutoff low 
	init_sid_field(&(sid_fields[3]),&(gPtr->sidValues->frr),         true, 33, 4, 0x00);//resonance
	init_sid_field(&(sid_fields[4]),&(gPtr->sidValues->frr),        false, 33, 5, 0x00);//filter mode
	init_sid_field(&(sid_fields[5]),&(gPtr->sidValues->maxVolume),   true, 33, 6, 0x00);//O3 off and band mode
	init_sid_field(&(sid_fields[6]),&(gPtr->sidValues->maxVolume),  false, 33, 7, 0x0C);//volume
	init_sid_field(&(sid_fields[7]),&(gPtr->sidValues->ctrl),        true, 33,10, 0x01);//control waveform
	init_sid_field(&(sid_fields[8]),&(gPtr->sidValues->ctrl),       false, 33,11, 0x00);//Ring sync gate
	init_sid_field(&(sid_fields[9]),&(gPtr->sidValues->ad),          true, 33,12, 0x01);//Attack
	init_sid_field(&(sid_fields[10]),&(gPtr->sidValues->ad),        false, 33,13, 0x09);//Decay
	init_sid_field(&(sid_fields[11]),&(gPtr->sidValues->sr),         true, 33,14, 0x05);//Sustain
	init_sid_field(&(sid_fields[12]),&(gPtr->sidValues->sr),        false, 33,15, 0x03);//Release
	init_sid_field(&(sid_fields[13]),&(gPtr->sidValues->pwdHi),     false, 33,16, 0x07);//Pulse width high
	init_sid_field(&(sid_fields[14]),&(gPtr->sidValues->pwdLo),      true, 34,16, 0x0F);//Pulse width low
	init_sid_field(&(sid_fields[15]),&(gPtr->sidValues->pwdLo),     false, 35,16, 0x0F);//Pulse width low
}

void fieldToChip(uint8_t which)
{
	uint8_t temp = sid_fields[which].value;
	if(sid_fields[which].isHighNib) temp = temp << 4; //put it in high nibble
	*(sid_fields[which].sidIPtr) = (*(sid_fields[which].sidIPtr)) | temp;
}

//title      : textSetColor(0x0F,0x04);
//sections   : textSetColor(0x0F,0x0A);
//normal     : textSetColor(0x0A,0x00);
//highlighted: textSetColor(0x0F,0x04);
//values:      textSetColor(0x0E,0x00);
void printInstrumentHeaders()
{
	uint8_t curLine = 2;


	textGotoXY(0,0);textSetColor(0x0F,0x04);textPrint("SID Tweak v0.1");
	
	textGotoXY(0,21);       
					        textPrint("[ESC]");
	textSetColor(0x0F,0x00);textPrint(" Quit ");
    textSetColor(0x0F,0x04);textPrint("[F1]");
	textSetColor(0x0F,0x00);textPrint(" Help ");
    textSetColor(0x0F,0x04);textPrint("[F3]");
	textSetColor(0x0F,0x00);textPrint(" Load ");
    textSetColor(0x0F,0x04);textPrint("[F5]");
	textSetColor(0x0F,0x00);textPrint(" Save ");
    textSetColor(0x0F,0x04);textPrint("[F7]");
	textSetColor(0x0F,0x00);textPrint(" Randomize!");
	
	textSetColor(0x0F,0x0A);
	textGotoXY(1,curLine++); printf("        SID Chip wide settings:");
	
	
	textSetColor(0x0A,0x00);
	textGotoXY(1,curLine++); printf("      Filter cuttoff (11 bits):");
	textGotoXY(1,curLine++); printf("                     Resonance:");
	textGotoXY(1,curLine++); printf("                   Filter Mode:");
	textGotoXY(1,curLine++); printf("                  O3,Band Mode:");
	textGotoXY(1,curLine++); printf("                        Volume:");
	
	curLine++;
	
	textSetColor(0x0F,0x0A);
	textGotoXY(1,curLine++); printf("                Voice settings:");
	
	textSetColor(0x0A,0x00);
	textGotoXY(1,curLine++); printf("                       Control:");
	textGotoXY(1,curLine++); printf("                Ring,Sync,Gate:");
	textGotoXY(1,curLine++); printf("                        Attack:");
	textGotoXY(1,curLine++); printf("                         Decay:");
	textGotoXY(1,curLine++); printf("                       Sustain:");
	textGotoXY(1,curLine++); printf("                       Release:");
	textGotoXY(1,curLine++); printf("         Pulse Width (12 bits):");
	
	
	curLine++;
	textSetColor(0x0F,0x0A);
	textGotoXY(1,curLine++); printf("             Current polyphony:");
	
}

void updateValues()
{
	uint8_t xAlign = 33;
	graphicsWaitVerticalBlank();
	
	textSetColor(0x0E,0x00);
	for(uint8_t i=0; i<16; i++)
		{
		if(i==indexUI) textSetColor(0x0F,0x04);
		else textSetColor(0x0E,0x00);
		if(sid_fields[i].is_dirty)
			{
			textGotoXY(sid_fields[i].tX, sid_fields[i].tY);printf("%01x",sid_fields[i].value);
			fieldToChip(i);
			sid_fields[i].is_dirty = false;
			}
		}
	textGotoXY(xAlign,18); 
	if((sid_fields[5].value & 0x80) == 0x80) textPrint("4");
	else textPrint("6");
	
	sid_StageOne();
}

void updateHighlight(uint8_t old, uint8_t nouv)
	{
	textSetColor(0x0E,0x00);textGotoXY(sid_fields[old].tX, sid_fields[old].tY);printf("%01x",sid_fields[old].value);
	textSetColor(0x0F,0x04);textGotoXY(sid_fields[nouv].tX, sid_fields[nouv].tY);printf("%01x",sid_fields[nouv].value);
	indexUI = nouv;
	}
}