#include "D:\F256\llvm-mos\code\NES\.builddir\trampoline.h"

#include "f256lib.h"
#include "../src/mupads.h"

void pollNES()
{
POKE(PAD_CTRL, CTRL_TRIG | CTRL_MODE_NES);
	asm("NOP");
	asm("NOP");
POKE(PAD_CTRL, CTRL_MODE_NES);
padPollDelayUntilReady();
}

void pollSNES()
{
POKE(PAD_CTRL, CTRL_TRIG | CTRL_MODE_SNES);
	asm("NOP");
	asm("NOP");
POKE(PAD_CTRL, CTRL_MODE_SNES);
padPollDelayUntilReady();
}

bool padPollIsReady()
{
return (PEEK(PAD_CTRL) & STAT_DONE); //if true, it's ready
}

void padPollDelayUntilReady()
{
while((PEEK(PAD_CTRL) & STAT_DONE) != STAT_DONE)
		;
}
