
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
	spriteDefine(gen->s, base+ val*UI_ACTIV, gen->size, 0, 0);
	spriteSetVisible(gen->s,true);
}

void updateRadioB(struct radioB_UI *but)
{
	if(but->gen.isClicked)
	{
		but->gen.addr += (uint32_t)UI_ACTIV;
	}
	else
	{
		but->gen.addr -= (uint32_t)UI_ACTIV;
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

void updateDial(struct dial_UI *dia, uint32_t base)
{
	
	dia->gen.addr = base + (uint32_t)UI_DIALS + (uint32_t)(getEigth((uint16_t)dia->value8, (uint16_t)dia->min8, (uint16_t)dia->max8) * UI_ACTIV);
	
	spriteDefine(dia->gen.s, dia->gen.addr, dia->gen.size, 0, 0);
	spriteSetPosition(dia->gen.s,dia->gen.x,dia->gen.y);
	
	spriteSetVisible(dia->gen.s  ,true);
	
}
void setDial(struct dial_UI *dia, uint8_t value8, uint8_t min8, uint8_t max8, uint16_t value16, uint16_t min16, uint16_t max16, uint32_t base)
{	
	dia->value8 = value8;
	dia->min8 = min8;
	dia->max8 = max8;
	dia->value16 = value16;
	dia->min16 = min16;
	dia->max16 = max16;
	
	updateDial(dia, base);

}

void updateSlider(struct slider_UI *sli, uint32_t base)
{
	sli->gen.addr = base + (uint32_t)UI_SLIDS + (uint32_t)(getEigth((uint16_t)sli->value8, (uint16_t)sli->min8, (uint16_t)sli->max8) * UI_ACTIV);
	
	spriteDefine(sli->gen.s, sli->gen.addr, sli->gen.size, 0, 0);
	spriteSetPosition(sli->gen.s,sli->gen.x,sli->gen.y);
	spriteDefine(sli->gen.s + 1, sli->gen.addr + UI_ROW, sli->gen.size, 0, 0);
	spriteSetPosition(sli->gen.s + 1,sli->gen.x,sli->gen.y + sli->gen.size);
	
	spriteSetVisible(sli->gen.s  ,true);
	spriteSetVisible(sli->gen.s+1,true);
	
}
void setSlider(struct slider_UI *sli, uint8_t value8, uint8_t min8, uint8_t max8, uint16_t value16, uint16_t min16, uint16_t max16, uint32_t base)
{	
	sli->value8 = value8;
	sli->min8 = min8;
	sli->max8 = max8;
	sli->value16 = value16;
	sli->min16 = min16;
	sli->max16 = max16;
	
	updateSlider(sli, base);
		
}
}