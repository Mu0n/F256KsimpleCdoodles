#include "f256lib.h"
#include "../src/muleds.h"

void enableManualLEDs(bool wantNet, bool wantLock, bool wantL1, bool wantL0, bool wantMedia, bool wantPower)
{
	POKE(SYS_CTRL_REG, (wantPower?SYS_PWR:0) | (wantMedia?SYS_SD:0) |
			           (wantLock?SYS_LCK:0)  | (wantNet?SYS_NET:0) |
					   (wantL0?SYS_L0:0) | (wantL1?SYS_L1:0));
}
}