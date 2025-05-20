
#include "f256lib.h"
#include "../src/moduUI.h"

uint8_t getEigth(uint16_t value, uint16_t min, uint16_t max) {
    if (value <= min) return 0;
    if (value >= max) return 7;

    uint8_t segmentSize = (max - min+1) / 8;
    return (uint8_t) ((value - min) / segmentSize);
}

void setGeneric(uint8_t s, uint16_t x, uint16_t y, uint32_t addr, 
uint8_t parentIndex, uint8_t size, uint8_t x1, uint8_t x2, uint8_t y1, uint8_t y2, uint8_t actionID, struct generic_UI *gen)
{
	gen->s = s;
	gen->x = x;
	gen->y = y;
	gen->isHovered = false;
	gen->isClicked = false;
	gen->isDisabled = false;
	gen->addr = addr;
	gen->parentIndex = parentIndex;
	gen->size = size;
	gen->x1 = x1;
	gen->x2 = x2;
	gen->y1 = y1;
	gen->y2 = y2;
	gen->actionID = actionID;
}


void showGeneric(struct generic_UI *gen)
{
	spriteDefine(gen->s, gen->addr, gen->size, 0, 0);
	spriteSetPosition(gen->s,gen->x,gen->y);
	spriteSetVisible(gen->s,true);
}
void updateGeneric(struct generic_UI *gen, uint8_t val, uint32_t base)
{
	spriteDefine(gen->s, base+ (uint32_t)(val*gen->size*gen->size), gen->size, 0, 0);
	spriteSetVisible(gen->s,true);
}

void updateRadioB(struct radioB_UI *but)
{
	if(but->gen.isClicked)
	{
		but->gen.addr += (uint32_t)(but->gen.size * but->gen.size);
	}
	else
	{
		but->gen.addr -= (uint32_t)(but->gen.size * but->gen.size);
	}
	
	spriteDefine(but->gen.s, but->gen.addr, but->gen.size, 0, 0);
	spriteSetVisible(but->gen.s,true);
}

void setRadioB(struct radioB_UI *but, bool isExcl, uint8_t groupID, bool startAct) { //assumes the generic part has been set first!

	but->isGroupExclusive = isExcl;
	but->groupID = groupID;
	
	spriteDefine(but->gen.s, but->gen.addr, but->gen.size, 0, 0);
	spriteSetPosition(but->gen.s,but->gen.x,but->gen.y);

	if(startAct) 
	{
		but->gen.isClicked = true;
		updateRadioB(but);
	}
	spriteSetVisible(but->gen.s,true);
}

void updateDial(struct dial_UI *dia, uint32_t base, uint32_t tileBase)
{
	uint8_t eigth = getEigth((uint16_t)dia->value8, (uint16_t)dia->min8, (uint16_t)dia->max8);
	uint8_t eigthC, eigthF;
	uint8_t absShift = 0;
	
	if(dia->shiftCoarse < 0) 
		{
		absShift = (uint8_t)(-1* dia->shiftCoarse);
		eigthC= (dia->value8 & dia->maskCoarse)<<absShift;
		}
	else 
	{
		absShift = dia->shiftCoarse;
		eigthC=(dia->value8 & dia->maskCoarse)>>absShift;
	}
	setANumber(&(dia->numCoarse), (uint16_t)(0x0F & eigthC), tileBase);
	if(dia->maskFine!=0x00)
		{
		if(dia->shiftFine < 0)
		{
			absShift = (uint8_t)(-1*dia->shiftFine);
			eigthF = (dia->value8 & dia->maskFine)<<absShift;
		}
		else
			{
			absShift = dia->shiftFine;
			eigthF= (dia->value8 & dia->maskFine)>>absShift;	
			}
		setANumber(&(dia->numFine), (uint16_t)(0x0F & eigthF), tileBase);
		}
	
	dia->gen.addr = base + (uint32_t)32*dia->gen.size*dia->gen.size + (uint32_t)(eigth * (uint32_t)dia->gen.size*dia->gen.size);
	
	spriteDefine(dia->gen.s, dia->gen.addr, dia->gen.size, 0, 0);
	spriteSetPosition(dia->gen.s,dia->gen.x,dia->gen.y);
	
	spriteSetVisible(dia->gen.s  ,true);
}
void setDial(struct dial_UI *dia, uint8_t value8, uint8_t min8, uint8_t max8, uint16_t value16, uint16_t min16, uint16_t max16, uint32_t base, uint32_t tileBase, uint8_t maskCoarse, int8_t shiftCoarse, uint8_t maskFine, int8_t shiftFine)
{	
	dia->value8 = value8;
	dia->min8 = min8;
	dia->max8 = max8;
	dia->value16 = value16;
	dia->min16 = min16;
	dia->max16 = max16;
	
	dia->maskCoarse = maskCoarse;
	dia->shiftCoarse = shiftCoarse;
	dia->maskFine = maskFine;
	dia->shiftFine = shiftFine;
	
	updateDial(dia, base,tileBase);
}

void updateSlider(struct slider_UI *sli, uint32_t base, uint32_t tileBase)
{
	uint8_t eigth = getEigth((uint16_t)sli->value8, (uint16_t)sli->min8, (uint16_t)sli->max8);
	sli->gen.addr = base + (uint32_t)(UI_SLIDS*sli->gen.size*sli->gen.size) + (uint32_t)(eigth *(uint16_t)(sli->gen.size* sli->gen.size));
	
	spriteDefine(sli->gen.s, sli->gen.addr, sli->gen.size, 0, 0);
	spriteSetPosition(sli->gen.s,sli->gen.x,sli->gen.y);
	spriteDefine(sli->gen.s + 1, sli->gen.addr + (uint32_t)(UI_ROW*sli->gen.size*sli->gen.size), sli->gen.size, 0, 0);
	spriteSetPosition(sli->gen.s + 1,sli->gen.x,sli->gen.y + sli->gen.size);
	
	spriteSetVisible(sli->gen.s  ,true);
	spriteSetVisible(sli->gen.s+1,true);
	
	setANumber(&(sli->num), (uint16_t)(0x0F & sli->value8), tileBase);
	
}
void setSlider(struct slider_UI *sli, uint8_t value8, uint8_t min8, uint8_t max8, uint16_t value16, uint16_t min16, uint16_t max16, uint32_t base, uint32_t tileBase)
{	
	sli->value8 = value8;
	sli->min8 = min8;
	sli->max8 = max8;
	sli->value16 = value16;
	sli->min16 = min16;
	sli->max16 = max16;
	
	updateSlider(sli, base, tileBase);
		
}

void updateLighter(struct lighter_UI *lit, uint32_t base)
{
	FAR_POKEW(base + (uint32_t)(2*lit->gen.x + 40*lit->gen.y), lit->tile); 
}
void setLighter(struct lighter_UI *lit, uint8_t tile, uint32_t base)
{
	lit->tile = tile;
	updateLighter(lit,base);
}
void updateNumber(struct number_UI *num, uint32_t base)
{
	FAR_POKEW(base + (uint32_t)(2*num->gen.x + 40*num->gen.y), num->tile); 
}

void setANumber(struct number_UI *num, uint16_t tile, uint32_t base)
{
	num->tile = tile;
	updateNumber(num, base);
}

void setATile(uint8_t x, uint8_t y, uint16_t tile, uint32_t base)
{
	FAR_POKEW(base + (uint32_t)(2*x + 40*y), tile); 
}
}