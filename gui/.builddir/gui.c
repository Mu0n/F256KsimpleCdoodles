#include "D:\F256\llvm-mos\code\gui\.builddir\trampoline.h"

#define F256LIB_IMPLEMENTATION

#define SPR_BASE 0x10000

#include "f256lib.h"
#include "../src/mumouse.h"
#include "../src/lGUI.h"

#define GUI_N_RADIOS 12
#define GUI_N_SLIDERS 8
#define GUI_N_DIALS 8

typedef struct sliderActivity{
	uint8_t index;
	int16_t iMX, iMY;
	uint8_t value8;
	uint16_t value16;
} sliderActivity;
typedef struct dialActivity{
	uint8_t index;
	uint8_t value8;
	uint16_t value16;
} dialActivity;

void dispatchAction(struct generic_UI *, bool);
void resetActivity(void);

EMBED(gui, "../assets/gui.bin", 0x10000); //4kb
EMBED(pal, "../assets/gui.pal", 0x11000); //1kb

int16_t mX, mY; //mouse coordinates

struct sliderActivity sliAct;
struct dialActivity   diaAct;

struct radioB_UI radios[12];
struct slider_UI sliders[8];
struct generic_UI sliders_labels[16];
struct dial_UI dials[8];


void loadGUI()
{
	
	
	//radio button group 0
	for(uint8_t i=0;i<4;i++)
	{
	setGeneric(i, 40 + i * 8, 40, SPR_BASE+UI_SWTCH, 0, 8, 0,6,2,5, i, &(radios[i].gen));
	setRadioB(&radios[i], true, i/4, (i%4==0)?true:false);
	}
	for(uint8_t i=4;i<8;i++)
	{
	setGeneric(i, 45 + i * 8, 40, SPR_BASE+UI_SWTCH, 0, 8, 0,6,2,5, i, &(radios[i].gen));
	setRadioB(&radios[i], true, i/4, (i%4==0)?true:false);
	}
	for(uint8_t i=8;i<12;i++)
	{
	setGeneric(i, 50 + i * 8, 40, SPR_BASE+UI_SWTCH, 0, 8, 0,6,2,5, i, &(radios[i].gen));
	setRadioB(&radios[i], true, i/4, (i%4==0)?true:false);
	}
	for(uint8_t i=12;i<16;i++)
	{
	setGeneric(i, 55 + i * 8, 40, SPR_BASE+UI_SWTCH, 0, 8, 0,6,2,5, i, &(radios[i].gen));
	setRadioB(&radios[i], true, i/4, (i%4==0)?true:false);
	}
	

	//sliders
	for(uint8_t i=0;i<GUI_N_SLIDERS;i++)
		{
		setGeneric(GUI_N_RADIOS + 2*i , 40 + i* 8 , 50, SPR_BASE+UI_SLIDS, 0, 8, 1,5,3,12, GUI_N_RADIOS + 2*i, &(sliders[i].gen));
		setSlider(&sliders[i], 0, 0, 15, 0, 0, 0, SPR_BASE);
	
		setGeneric(i+GUI_N_RADIOS+2*GUI_N_SLIDERS, 40 + i * 8, 66, SPR_BASE, 0, 8, 0,0,0,0, i+GUI_N_RADIOS+2*GUI_N_SLIDERS, &(sliders_labels[i]));
		showGeneric(&(sliders_labels[i]));
		updateGeneric(&(sliders_labels[i]), sliders[i].value8, SPR_BASE);
		}
		/*
	//dials
	for(uint8_t i=0; i<GUI_N_DIALS; i++)
	{
		setGeneric(i+GUI_N_RADIOS+3*GUI_N_SLIDERS, 40 + i* 8 , 82, SPR_BASE+UI_DIALS, 0, 8, 1,5,3,12, i+GUI_N_RADIOS+3*GUI_N_SLIDERS, &(dials[i].gen));
		setDial(&(dials[i]), 0x20, 0, 0x7F, 0, 0, 0, SPR_BASE);
	
		setGeneric(i+GUI_N_RADIOS+3*GUI_N_SLIDERS+GUI_N_DIALS, 40 + i * 8, 98, SPR_BASE, 0, 8, 0,0,0,0, i+GUI_N_RADIOS+3*GUI_N_SLIDERS+GUI_N_DIALS, &(sliders_labels[i+GUI_N_DIALS]));
		showGeneric(&(sliders_labels[i+GUI_N_DIALS]));
		updateGeneric(&(sliders_labels[i+GUI_N_DIALS]), sliders[i+GUI_N_DIALS].value8, SPR_BASE);
	}
	*/
}

void backgroundSetup()
{
	uint16_t c;
	
	POKE(MMU_IO_CTRL, 0x00);
	// XXX GAMMA  SPRITE   TILE  | BITMAP  GRAPH  OVRLY  TEXT
	POKE(VKY_MSTR_CTRL_0, 0b00101111); //sprite,graph,overlay,text
	// XXX XXX  FON_SET FON_OVLY | MON_SLP DBL_Y  DBL_X  CLK_70
	POKE(VKY_MSTR_CTRL_1, 0b00000100); //font overlay, double height text, 320x240 at 60 Hz;
	POKE(VKY_LAYER_CTRL_0, 0b00000001); //bitmap 1 in layer 0, bitmap 0 in layer 1
	POKE(VKY_LAYER_CTRL_1, 0b00000010); //bitmap 2 in layer 2

	POKE(0xD00D,0x00); //force black graphics background
	POKE(0xD00E,0x00);
	POKE(0xD00F,0x00);
//palette
	POKE(MMU_IO_CTRL,1);  //MMU I/O to page 1
	//prep to copy over the palette to the CLUT
	for(c=0;c<1023;c++)
	{
		POKE(VKY_GR_CLUT_0+c, FAR_PEEK(0x11000+c)); //palette for GUI
	}
	
	POKE(MMU_IO_CTRL,0); //MMU I/O to page 0

	
	bitmapSetActive(0);
	bitmapSetCLUT(0);
	bitmapSetColor(3);
	bitmapClear();
	
	bitmapSetVisible(0,true);
	bitmapSetVisible(1,false);
	bitmapSetVisible(2,false);
}

void resetActivity()
{
	diaAct.index = 0xFF;
	diaAct.value8 = 0;
	diaAct.value16 = 0;
	sliAct.index = 0xFF;
	sliAct.value8 = 0;
	sliAct.value16 = 0;
}
void setup()
	{
	backgroundSetup();
	prepMouse();
	loadGUI();
	resetActivity();
	}

void dispatchAction(struct generic_UI *gen, bool isClicked) //here we dispatch what the click behavior means in this specific project
{
	switch(gen->actionID)
	{
		case 0 ... GUI_N_RADIOS-1:
			gen->isClicked = !gen->isClicked;
			for(uint8_t i=0; i<GUI_N_RADIOS; i++)
			{
				if(gen->s == radios[i].gen.s) //found it
				{
					updateRadioB(&radios[i]); //toggle the current one
					if(gen->isClicked && radios[i].isGroupExclusive) //start toggling others off if it was excl
					{
						for(uint8_t j=0; j<GUI_N_RADIOS; j++)
						{
							if(i==j) continue; //leave the current one alone
							if(radios[j].isGroupExclusive && radios[i].groupID == radios[j].groupID) //if they are part of the same group
								{
									if(radios[j].gen.isClicked)
									{
									radios[j].gen.isClicked = false;
									updateRadioB(&radios[j]);
									}
								}
						}
					}
				}
			}
			break;
	
		case GUI_N_RADIOS ... (GUI_N_RADIOS+2*GUI_N_SLIDERS-1):
			gen->isClicked = true;
			for(uint8_t i=0; i<GUI_N_SLIDERS; i++)
			{
				if(gen->s == sliders[i].gen.s) //found it
				{
				sliAct.iMX = mX;
				sliAct.iMY = mY;
				sliAct.index = i;
				hideMouse();
				}
			}
			break;

		case (GUI_N_RADIOS+2*GUI_N_SLIDERS) ... (GUI_N_RADIOS+2*GUI_N_SLIDERS+GUI_N_DIALS-1):
			break;
	}
}

void checkUIEClick(uint16_t newX, uint16_t newY, struct generic_UI *gen) //uses the generic part to verify if it was clicked
{

	if(newX >= (gen->x + gen->x1) && newX <= (gen->x + gen->x2))
	{
		if(newY >= (gen->y + gen->y1) && newY <= (gen->y + gen->y2)) //multi stage check to speed it up
		{
			dispatchAction(gen, gen->isClicked);
		}
	}

}


void checkUIClicks(uint16_t newX,uint16_t newY) //parse every clickable element
{
	for(uint8_t i=0; i<GUI_N_RADIOS; i++)
	{
		checkUIEClick(newX, newY, &(radios[i].gen)); //check this group of radio buttons
	}
	for(uint8_t i=0; i<GUI_N_SLIDERS; i++)
	{
		checkUIEClick(newX, newY, &(sliders[i].gen)); //check this group of sliders
	}
}

	
int main(int argc, char *argv[]) {
bool mPressed = false; //current latch status of left mouse click. if you long press, it won't trigger many multiples of the event

setup();

	while(true)
	{

	//events
	kernelNextEvent();
	if(kernelEventData.type == kernelEvent(mouse.CLICKS))
	{
		//dispatchNote(bool isOn, uint8_t channel, uint8_t note, uint8_t speed, bool wantAlt, uint8_t whichChip, bool isBeat, uint8_t beatChan)
	}
	else if(kernelEventData.type == kernelEvent(mouse.DELTA))
	{
		mX = PEEKW(PS2_M_X_LO)+(int8_t)kernelEventData.mouse.delta.x;
		mY = PEEKW(PS2_M_Y_LO)+(int8_t)kernelEventData.mouse.delta.y;
	
		if(mX<0) mX=0; if(mX>640-16) mX=640-16;
		if(mY<0) mY=0; if(mY>480-16) mY=480-16;
		POKEW(PS2_M_X_LO,mX);
        POKEW(PS2_M_Y_LO,mY);

		if(sliAct.index != 0xFF)  //keep track of slider technology
			{
				int16_t temp;
				temp = (int16_t)sliders[sliAct.index].value8;
				temp -= (int8_t)kernelEventData.mouse.delta.y;
				if(temp < (int16_t)sliders[sliAct.index].min8) temp = sliders[sliAct.index].min8;
				if(temp > (int16_t)sliders[sliAct.index].max8) temp = sliders[sliAct.index].max8;
	
				sliders[sliAct.index].value8 = (uint8_t)temp;
				updateSlider(&(sliders[sliAct.index]),SPR_BASE);
				updateGeneric(&(sliders_labels[sliAct.index]),sliders[sliAct.index].value8,SPR_BASE);
			}
	
		if((kernelEventData.mouse.delta.buttons&0x01)==0x01 && mPressed==false)
			{
				mPressed=true; //activate the latch
				checkUIClicks((mX>>1)+32,(mY>>1)+32);
			}
		if(mPressed==true && (kernelEventData.mouse.delta.buttons&0x01)==0x00)
			{
				if(sliAct.index != 0xFF)
					{
						sliders[sliAct.index].gen.isClicked = false;
						sliAct.index = 0xFF;
						showMouse();
					}
				mPressed=false; //release the latch
			}
		}
	}
	
return 0;}
