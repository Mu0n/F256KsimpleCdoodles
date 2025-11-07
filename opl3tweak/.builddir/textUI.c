#include "D:\F256\llvm-mos\code\opl3tweak\.builddir\trampoline.h"

#include "f256lib.h"
#include "../src/muopl3.h"
#include "../src/mudispatch.h"
#include "../src/textUI.h"


struct OPL3Field opl3_fields[22];
uint8_t indexUI = 0;

uint8_t navWSJumps[22] = {0x21,
						  0x11,
						  0x12,0x22,
						  0x22,0x22,
						  0x22,0x22,
						  0x22,0x22,
						  0x23,0x23,
				     0x24,0x33,0x33,0x42,
						  0x32,0x32,0x22,0x22,0x22,0x21
						  }; //high nibble how to jump back, low nibble how to jump forward while navigating with W and S
void init_opl3_field(struct OPL3Field *f, uint8_t *target, bool highNib, uint8_t x, uint8_t y, uint8_t startVal) {
    f->value = startVal;
    f->opl3IPtr = target;
    f->isHighNib = highNib;
    f->is_dirty = true;
    f->tX = x;
    f->tY = y;
}

void randomInst()
{
	
opl3_fields[0].value=  (uint8_t)randomRead()&0x0F;  //Tremo/Vibra/Perc Mode
opl3_fields[1].value=  (uint8_t)randomRead()&0x0F;  //Feedback/Algorithm...
opl3_fields[2].value=  (uint8_t)randomRead()&0x03;  //Waveform mod ........
opl3_fields[3].value=  (uint8_t)randomRead()&0x03;  //Waveform car.........
opl3_fields[4].value=  (uint8_t)randomRead()&0x0F;  //Attack mod...........
opl3_fields[5].value=  (uint8_t)randomRead()&0x0F;  //Attack car...........
opl3_fields[6].value=  (uint8_t)randomRead()&0x0F;  //Decay  mod...........
opl3_fields[7].value=  (uint8_t)randomRead()&0x0F;  //Decay  car...........
opl3_fields[8].value=  (uint8_t)randomRead()&0x0F;  //Sustain mod..........
opl3_fields[9].value=  (uint8_t)randomRead()&0x0F;  //Sustain car..........
opl3_fields[10].value= (uint8_t)randomRead()&0x0F;  //Release mod..........
opl3_fields[11].value= (uint8_t)randomRead()&0x0F;  //Release car..........
//opl3_fields[12].value= (uint8_t)randomRead()&0x03;  //Volume mod 2msb......
//opl3_fields[13].value= (uint8_t)randomRead()&0x0F;  //Volume mod 4lsb......
//opl3_fields[14].value= (uint8_t)randomRead()&0x03;  //Volume car 2msb......
//opl3_fields[15].value= (uint8_t)randomRead()&0x0F;  //Volume car 4lsb......
opl3_fields[16].value= (uint8_t)randomRead()&0x03;  //Volume Scaling mod...
opl3_fields[17].value= (uint8_t)randomRead()&0x03;  //Volume Scaling car...
opl3_fields[18].value= (uint8_t)randomRead()&0x0F;  //T/V/ET/ES mod........
opl3_fields[19].value= (uint8_t)randomRead()&0x0F;  //T/V/ET/ES car........
opl3_fields[20].value= (uint8_t)randomRead()&0x0F;  //Freq Mult mod........
opl3_fields[21].value= (uint8_t)randomRead()&0x0F;  //Freq Mult car........

for(uint8_t i=0; i<MAXUIINDEX; i++)opl3_fields[i].is_dirty=true;
updateValues();
	
}


void initSIDFields()
{
	init_opl3_field(&(opl3_fields[0]), &(gPtr->OPL3Values->VT_DEPTH),      true, 34, 3, 0x00);//Tremo/Vibra/Perc Mode
	init_opl3_field(&(opl3_fields[1]), &(gPtr->OPL3Values->CHAN_FEED),    false, 34, 6, 0x08);//Feedback/Algorithm...
	init_opl3_field(&(opl3_fields[2]), &(gPtr->OPL3Values->OP2_WAV),      false, 34, 9, 0x00);//Waveform mod ........
	init_opl3_field(&(opl3_fields[3]), &(gPtr->OPL3Values->OP1_WAV),      false, 35, 9, 0x01);//Waveform car.........
	init_opl3_field(&(opl3_fields[4]), &(gPtr->OPL3Values->OP2_AD),        true, 34,10, 0x05);//Attack mod...........
	init_opl3_field(&(opl3_fields[5]), &(gPtr->OPL3Values->OP1_AD),        true, 35,10, 0x08);//Attack car...........
	init_opl3_field(&(opl3_fields[6]), &(gPtr->OPL3Values->OP2_AD),       false, 34,11, 0x01);//Decay  mod...........
	init_opl3_field(&(opl3_fields[7]), &(gPtr->OPL3Values->OP1_AD),       false, 35,11, 0x03);//Decay  car...........
	init_opl3_field(&(opl3_fields[8]), &(gPtr->OPL3Values->OP2_SR),        true, 34,12, 0x08);//Sustain mod..........
	init_opl3_field(&(opl3_fields[9]), &(gPtr->OPL3Values->OP1_SR),        true, 35,12, 0x08);//Sustain car..........
	init_opl3_field(&(opl3_fields[10]),&(gPtr->OPL3Values->OP2_SR),       false, 34,13, 0x08);//Release mod..........
	init_opl3_field(&(opl3_fields[11]),&(gPtr->OPL3Values->OP1_SR),       false, 35,13, 0x08);//Release car..........
	init_opl3_field(&(opl3_fields[12]),&(gPtr->OPL3Values->OP2_KSLVOL),    true, 33,14, 0x00);//Volume mod 2msb......
	init_opl3_field(&(opl3_fields[13]),&(gPtr->OPL3Values->OP2_KSLVOL),   false, 34,14, 0x08);//Volume mod 4lsb......
	init_opl3_field(&(opl3_fields[14]),&(gPtr->OPL3Values->OP1_KSLVOL),    true, 35,14, 0x00);//Volume car 2msb......
	init_opl3_field(&(opl3_fields[15]),&(gPtr->OPL3Values->OP1_KSLVOL),   false, 36,14, 0x08);//Volume car 4lsb......
	init_opl3_field(&(opl3_fields[16]),&(gPtr->OPL3Values->OP2_KSLVOL),    true, 34,15, 0x00);//Volume Scaling mod...
	init_opl3_field(&(opl3_fields[17]),&(gPtr->OPL3Values->OP1_KSLVOL),    true, 35,15, 0x00);//Volume Scaling car...
	init_opl3_field(&(opl3_fields[18]),&(gPtr->OPL3Values->OP2_TVSKF),     true, 34,16, 0x02);//T/V/ET/ES mod........
	init_opl3_field(&(opl3_fields[19]),&(gPtr->OPL3Values->OP1_TVSKF),     true, 35,16, 0x02);//T/V/ET/ES car........
	init_opl3_field(&(opl3_fields[20]),&(gPtr->OPL3Values->OP2_TVSKF),    false, 34,17, 0x02);//Freq Mult mod........
	init_opl3_field(&(opl3_fields[21]),&(gPtr->OPL3Values->OP1_TVSKF),    false, 35,17, 0x02);//Freq Mult car........
}

void fieldToChip(uint8_t which)
{
	uint8_t temp = opl3_fields[which].value;
	if(opl3_fields[which].isHighNib) temp = temp << 4; //put it in high nibble
	*(opl3_fields[which].opl3IPtr) = (*(opl3_fields[which].opl3IPtr)) | temp;
}

//title      : textSetColor(0x0F,0x04);
//sections   : textSetColor(0x0F,0x0A);
//normal     : textSetColor(0x0A,0x00);
//highlighted: textSetColor(0x0F,0x04);
//values:      textSetColor(0x0E,0x00);
void printInstrumentHeaders()
{
	uint8_t curLine = 2;


	textGotoXY(0,0);textSetColor(0x0F,0x03);textPrint("OPL3 Tweak v0.1");
	
	textGotoXY(0,21);
					        textPrint("[ESC]");
	textSetColor(0x0F,0x00);textPrint(" Quit ");
    textSetColor(0x0F,0x03);textPrint("[F1]");
	textSetColor(0x0F,0x00);textPrint(" Help ");
    textSetColor(0x0F,0x03);textPrint("[F3]");
	textSetColor(0x0F,0x00);textPrint(" Load ");
    textSetColor(0x0F,0x03);textPrint("[F5]");
	textSetColor(0x0F,0x00);textPrint(" Save ");
    textSetColor(0x0F,0x03);textPrint("[F7]");
	textSetColor(0x0F,0x00);textPrint(" Randomize!");
	
	textSetColor(0x0F,0x0A);
	textGotoXY(1,curLine++); printf("       OPL3 Chip wide settings:");
	
	textSetColor(0x0A,0x00);
	textGotoXY(1,curLine++); printf("         Tremo/Vibra/Perc Mode:");
	
	curLine++;
	
	textSetColor(0x0F,0x0A);
	textGotoXY(1,curLine++); printf("              Channel settings:");
	
	textSetColor(0x0A,0x00);
	textGotoXY(1,curLine++); printf("            Feedback/Algorithm:");
	
	
	curLine++;
	
	textSetColor(0x0F,0x0A);
	textGotoXY(1,curLine++); printf(" Modulator-Carrier Op settings:");
	
	textSetColor(0x0A,0x00);
	textGotoXY(1,curLine++); printf("                      Waveform:");
	textGotoXY(1,curLine++); printf("                        Attack:");
	textGotoXY(1,curLine++); printf("                         Decay:");
	textGotoXY(1,curLine++); printf("                       Sustain:");
	textGotoXY(1,curLine++); printf("                       Release:");
	textGotoXY(1,curLine++); printf("                        Volume:");
	textGotoXY(1,curLine++); printf("                Volume Scaling:");
	textGotoXY(1,curLine++); printf("  Tremo/Vibra/EnvType/EnvScale:");
	textGotoXY(1,curLine++); printf("               Freq Multiplier:");
	
	
	curLine++;
	textSetColor(0x0F,0x0A);
	textGotoXY(1,curLine++); printf("             Current polyphony:");
	
}

void updateValues()
{
	graphicsWaitVerticalBlank();
	
	textSetColor(0x0E,0x00);
	for(uint8_t i=0; i<MAXUIINDEX; i++)
		{
		if(i==indexUI) textSetColor(0x0F,0x03);
		else textSetColor(0x03,0x00);
		if(opl3_fields[i].is_dirty)
			{
			textGotoXY(opl3_fields[i].tX, opl3_fields[i].tY);printf("%01x",opl3_fields[i].value);
			fieldToChip(i);
			opl3_fields[i].is_dirty = false;
			}
		}
	textGotoXY(34,19);
	if((opl3_fields[5].value & 0x80) == 0x80) textPrint("4");
	else textPrint("18");
	
	opl3_StageOne();
}

void updateHighlight(uint8_t old, uint8_t nouv)
	{
	textSetColor(0x03,0x00);textGotoXY(opl3_fields[old].tX, opl3_fields[old].tY);printf("%01x",opl3_fields[old].value);
	textSetColor(0x0F,0x03);textGotoXY(opl3_fields[nouv].tX, opl3_fields[nouv].tY);printf("%01x",opl3_fields[nouv].value);
	indexUI = nouv;
	}
	
	

