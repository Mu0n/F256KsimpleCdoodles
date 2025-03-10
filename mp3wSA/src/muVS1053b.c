#include "f256lib.h"
#include "../src/muVS1053b.h"
#include "../src/muUtils.h"
//The VS1053b is present on the Jr2 and K2.

const uint16_t rtmplugin[28] = { /* Compressed plugin  for the VS1053b to enable real time midi mode */
  0x0007, 0x0001, 0x8050, 0x0006, 0x0014, 0x0030, 0x0715, 0xb080, /*    0 */
  0x3400, 0x0007, 0x9255, 0x3d00, 0x0024, 0x0030, 0x0295, 0x6890, /*    8 */
  0x3400, 0x0030, 0x0495, 0x3d00, 0x0024, 0x2908, 0x4d40, 0x0030, /*   10 */
  0x0200, 0x000a, 0x0001, 0x0050
};

const uint16_t saplugin[950] = { /* Compressed plugin  for the VS1053b to enable the default spectrum analyzer values in 14 bands */
  0x0007, 0x0001, 0x8050, 0x0006, 0x0018, 0x3613, 0x0024, 0x3e00, /*    0 */
  0x3801, 0x0000, 0x16d7, 0xf400, 0x55c0, 0x0000, 0x0c17, 0xf400, /*    8 */
  0x57c0, 0x0007, 0x9257, 0xb080, 0x0024, 0x3f00, 0x0024, 0x2000, /*   10 */
  0x0000, 0x36f0, 0x1801, 0x2800, 0x31c0, 0x0007, 0x0001, 0x805c, /*   18 */
  0x0006, 0x00d6, 0x3e12, 0xb817, 0x3e12, 0x3815, 0x3e05, 0xb814, /*   20 */
  0x3615, 0x0024, 0x0000, 0x800a, 0x3e10, 0x3801, 0x0006, 0x0000, /*   28 */
  0x3e10, 0xb803, 0x0000, 0x0303, 0x3e11, 0x3805, 0x3e11, 0xb807, /*   30 */
  0x3e14, 0x3812, 0xb884, 0x130c, 0x3410, 0x4024, 0x4112, 0x10d0, /*   38 */
  0x4010, 0x008c, 0x4010, 0x0024, 0xf400, 0x4012, 0x3000, 0x3840, /*   40 */
  0x3009, 0x3801, 0x0000, 0x0041, 0xfe02, 0x0024, 0x2900, 0x8200, /*   48 */
  0x48b2, 0x0024, 0x36f3, 0x0844, 0x6306, 0x8845, 0xae3a, 0x8840, /*   50 */
  0xbf8e, 0x8b41, 0xac32, 0xa846, 0xffc8, 0xabc7, 0x3e01, 0x7800, /*   58 */
  0xf400, 0x4480, 0x6090, 0x0024, 0x6090, 0x0024, 0xf400, 0x4015, /*   60 */
  0x3009, 0x3446, 0x3009, 0x37c7, 0x3009, 0x1800, 0x3009, 0x3844, /*   68 */
  0x48b3, 0xe1e0, 0x4882, 0x4040, 0xfeca, 0x0024, 0x5ac2, 0x0024, /*   70 */
  0x5a52, 0x0024, 0x4cc2, 0x0024, 0x48ba, 0x4040, 0x4eea, 0x4801, /*   78 */
  0x4eca, 0x9800, 0xff80, 0x1bc1, 0xf1eb, 0xe3e2, 0xf1ea, 0x184c, /*   80 */
  0x4c8b, 0xe5e4, 0x48be, 0x9804, 0x488e, 0x41c6, 0xfe82, 0x0024, /*   88 */
  0x5a8e, 0x0024, 0x525e, 0x1b85, 0x4ffe, 0x0024, 0x48b6, 0x41c6, /*   90 */
  0x4dd6, 0x48c7, 0x4df6, 0x0024, 0xf1d6, 0x0024, 0xf1d6, 0x0024, /*   98 */
  0x4eda, 0x0024, 0x0000, 0x0fc3, 0x2900, 0x8200, 0x4e82, 0x0024, /*   a0 */
  0x4084, 0x130c, 0x0006, 0x0100, 0x3440, 0x4024, 0x4010, 0x0024, /*   a8 */
  0xf400, 0x4012, 0x3200, 0x4024, 0xb132, 0x0024, 0x4214, 0x0024, /*   b0 */
  0xf224, 0x0024, 0x6230, 0x0024, 0x0001, 0x0001, 0x2800, 0x2b49, /*   b8 */
  0x0000, 0x0024, 0xf400, 0x40c2, 0x3200, 0x0024, 0xff82, 0x0024, /*   c0 */
  0x48b2, 0x0024, 0xb130, 0x0024, 0x6202, 0x0024, 0x003f, 0xf001, /*   c8 */
  0x2800, 0x2e51, 0x0000, 0x1046, 0xfe64, 0x0024, 0x48be, 0x0024, /*   d0 */
  0x2800, 0x2f40, 0x3a01, 0x8024, 0x3200, 0x0024, 0xb010, 0x0024, /*   d8 */
  0xc020, 0x0024, 0x3a00, 0x0024, 0x36f4, 0x1812, 0x36f1, 0x9807, /*   e0 */
  0x36f1, 0x1805, 0x36f0, 0x9803, 0x36f0, 0x1801, 0x3405, 0x9014, /*   e8 */
  0x36f3, 0x0024, 0x36f2, 0x1815, 0x2000, 0x0000, 0x36f2, 0x9817, /*   f0 */
  0x0007, 0x0001, 0x80c7, 0x0006, 0x01ae, 0x3e12, 0xb817, 0x3e12, /*   f8 */
  0x3815, 0x3e05, 0xb814, 0x3625, 0x0024, 0x0000, 0x800a, 0x3e10, /*  100 */
  0x7802, 0x3e10, 0xf804, 0x3e11, 0x7810, 0x3e14, 0x7813, 0x0006, /*  108 */
  0x0051, 0x3e13, 0xf80e, 0x3e13, 0x4024, 0x3009, 0x3840, 0x3009, /*  110 */
  0x3852, 0x2911, 0xf140, 0x0006, 0x06d0, 0x3100, 0x5bd2, 0x0006, /*  118 */
  0xc351, 0x3009, 0x1bc0, 0x3009, 0x0402, 0x6126, 0x0024, 0x0006, /*  120 */
  0x00d1, 0x2800, 0x4d45, 0x0000, 0x0024, 0x0006, 0x0011, 0xb882, /*  128 */
  0x184c, 0x3009, 0x3850, 0x0006, 0x0010, 0x3009, 0x3800, 0x2914, /*  130 */
  0xbec0, 0x0000, 0x1800, 0x0006, 0x0010, 0xb882, 0x0024, 0x2915, /*  138 */
  0x7ac0, 0x0000, 0x1700, 0x0000, 0x0301, 0x3900, 0x5bc0, 0x0006, /*  140 */
  0xc351, 0x3009, 0x1bd0, 0x3009, 0x0404, 0x0006, 0x0051, 0x2800, /*  148 */
  0x3d40, 0x3901, 0x0024, 0x4448, 0x0401, 0x4192, 0x0024, 0x6498, /*  150 */
  0x2401, 0x001f, 0x4001, 0x6412, 0x0024, 0x0006, 0x0011, 0x2800, /*  158 */
  0x3c91, 0x0000, 0x058e, 0x2400, 0x4c4e, 0x0000, 0x0013, 0x0006, /*  160 */
  0x0051, 0x0006, 0x1a03, 0x3100, 0x4024, 0xf212, 0x44c4, 0x4346, /*  168 */
  0x0024, 0xf400, 0x40d5, 0x3500, 0x8024, 0x612a, 0x0024, 0x0000, /*  170 */
  0x0024, 0x2800, 0x4c91, 0x0000, 0x0024, 0x3613, 0x0024, 0x3100, /*  178 */
  0x3800, 0x2915, 0x7dc0, 0xf200, 0x0024, 0x003f, 0xfec2, 0x4082, /*  180 */
  0x4411, 0x3113, 0x1bc0, 0xa122, 0x0024, 0x0000, 0x2002, 0x6124, /*  188 */
  0x2401, 0x0000, 0x1002, 0x2800, 0x4648, 0x0000, 0x0024, 0x003f, /*  190 */
  0xf802, 0x3100, 0x4024, 0xb124, 0x0024, 0x2800, 0x4c00, 0x3900, /*  198 */
  0x8024, 0x6124, 0x0024, 0x0000, 0x0802, 0x2800, 0x4888, 0x0000, /*  1a0 */
  0x0024, 0x003f, 0xfe02, 0x3100, 0x4024, 0xb124, 0x0024, 0x2800, /*  1a8 */
  0x4c00, 0x3900, 0x8024, 0x6124, 0x0024, 0x0000, 0x0402, 0x2800, /*  1b0 */
  0x4ac8, 0x0000, 0x0024, 0x003f, 0xff02, 0x3100, 0x4024, 0xb124, /*  1b8 */
  0x0024, 0x2800, 0x4c00, 0x3900, 0x8024, 0x6124, 0x0401, 0x003f, /*  1c0 */
  0xff82, 0x2800, 0x4c08, 0xb124, 0x0024, 0x3900, 0x8024, 0xb882, /*  1c8 */
  0x8c4c, 0x3830, 0x4024, 0x0006, 0x0091, 0x3904, 0xc024, 0x0006, /*  1d0 */
  0x00d1, 0x0000, 0x0013, 0x3100, 0x904c, 0x4202, 0x0024, 0x39f0, /*  1d8 */
  0x4024, 0x3100, 0x4024, 0x3c00, 0x4024, 0xf400, 0x44c1, 0x34f0, /*  1e0 */
  0x8024, 0x6126, 0x0024, 0x0006, 0x06d0, 0x2800, 0x5b98, 0x4294, /*  1e8 */
  0x0024, 0x2400, 0x5b42, 0x0000, 0x0024, 0xf400, 0x4411, 0x3123, /*  1f0 */
  0x0024, 0x3100, 0x8024, 0x4202, 0x0024, 0x4182, 0x2401, 0x0000, /*  1f8 */
  0x2002, 0x2800, 0x5b49, 0x0000, 0x0024, 0x3013, 0x184c, 0x30f0, /*  200 */
  0x7852, 0x6124, 0xb850, 0x0006, 0x0001, 0x2800, 0x55c8, 0x4088, /*  208 */
  0x44c2, 0x4224, 0x0024, 0x4122, 0x0024, 0x4122, 0x0024, 0xf400, /*  210 */
  0x4051, 0x2900, 0x7200, 0x0000, 0x5708, 0x4224, 0x0024, 0x4122, /*  218 */
  0x0024, 0x4122, 0x0024, 0x2900, 0x6780, 0xf400, 0x4051, 0x0002, /*  220 */
  0x0002, 0x3009, 0x1bd0, 0x3023, 0x1bd2, 0x30e0, 0x4024, 0x6124, /*  228 */
  0x0024, 0x0000, 0x0024, 0x2800, 0x5b48, 0x0000, 0x0024, 0x3613, /*  230 */
  0x0024, 0x3e14, 0xc024, 0x2900, 0x1700, 0x3e14, 0x0024, 0x36e3, /*  238 */
  0x008c, 0x30e0, 0x8024, 0x6822, 0x4411, 0x3123, 0x0024, 0x3900, /*  240 */
  0x4024, 0x3033, 0x0c4c, 0x0006, 0x0011, 0x6892, 0x04c2, 0xa122, /*  248 */
  0x0402, 0x6126, 0x0024, 0x0006, 0x0093, 0x2800, 0x64c1, 0x0000, /*  250 */
  0x0024, 0xb882, 0x184c, 0x3413, 0x3812, 0x0006, 0x00d2, 0x3a00, /*  258 */
  0x5bd2, 0x3300, 0x4024, 0x0000, 0x0013, 0x3c00, 0x4024, 0xf400, /*  260 */
  0x44c1, 0x34f0, 0x8024, 0x6126, 0x0024, 0x0006, 0x0111, 0x2800, /*  268 */
  0x64d8, 0x4294, 0x0024, 0x2400, 0x6482, 0x0000, 0x0024, 0x0003, /*  270 */
  0xf001, 0x3101, 0x0024, 0xb412, 0x0024, 0x0028, 0x0001, 0x2800, /*  278 */
  0x6485, 0x6144, 0x0024, 0x0004, 0x0002, 0x2800, 0x6441, 0x4422, /*  280 */
  0x0024, 0x0000, 0x1002, 0x6422, 0x0024, 0x2800, 0x6480, 0x3900, /*  288 */
  0x4024, 0x3900, 0x4024, 0x3113, 0x0c4c, 0x36f3, 0x4024, 0x36f3, /*  290 */
  0xd80e, 0x36f4, 0x5813, 0x36f1, 0x5810, 0x36f0, 0xd804, 0x36f0, /*  298 */
  0x5802, 0x3405, 0x9014, 0x36f3, 0x0024, 0x36f2, 0x1815, 0x2000, /*  2a0 */
  0x0000, 0x36f2, 0x9817, 0x0007, 0x0001, 0x1868, 0x0006, 0x000f, /*  2a8 */
  0x0032, 0x004f, 0x007e, 0x00c8, 0x013d, 0x01f8, 0x0320, 0x04f6, /*  2b0 */
  0x07e0, 0x0c80, 0x13d8, 0x1f7f, 0x3200, 0x4f5f, 0x61a8, 0x0006, /*  2b8 */
  0x8008, 0x0000, 0x0007, 0x0001, 0x819e, 0x0006, 0x0054, 0x3e12, /*  2c0 */
  0xb814, 0x0000, 0x800a, 0x3e10, 0x3801, 0x3e10, 0xb803, 0x3e11, /*  2c8 */
  0x7806, 0x3e11, 0xf813, 0x3e13, 0xf80e, 0x3e13, 0x4024, 0x3e04, /*  2d0 */
  0x7810, 0x449a, 0x0040, 0x0001, 0x0003, 0x2800, 0x70c4, 0x4036, /*  2d8 */
  0x03c1, 0x0003, 0xffc2, 0xb326, 0x0024, 0x0018, 0x0042, 0x4326, /*  2e0 */
  0x4495, 0x4024, 0x40d2, 0x0000, 0x0180, 0xa100, 0x4090, 0x0010, /*  2e8 */
  0x0fc2, 0x4204, 0x0024, 0xbc82, 0x4091, 0x459a, 0x0024, 0x0000, /*  2f0 */
  0x0054, 0x2800, 0x6fc4, 0xbd86, 0x4093, 0x2400, 0x6f85, 0xfe01, /*  2f8 */
  0x5e0c, 0x5c43, 0x5f2d, 0x5e46, 0x020c, 0x5c56, 0x8a0c, 0x5e53, /*  300 */
  0x5e0c, 0x5c43, 0x5f2d, 0x5e46, 0x020c, 0x5c56, 0x8a0c, 0x5e52, /*  308 */
  0x0024, 0x4cb2, 0x4405, 0x0018, 0x0044, 0x654a, 0x0024, 0x2800, /*  310 */
  0x7dc0, 0x36f4, 0x5810, 0x0007, 0x0001, 0x81c8, 0x0006, 0x0080, /*  318 */
  0x3e12, 0xb814, 0x0000, 0x800a, 0x3e10, 0x3801, 0x3e10, 0xb803, /*  320 */
  0x3e11, 0x7806, 0x3e11, 0xf813, 0x3e13, 0xf80e, 0x3e13, 0x4024, /*  328 */
  0x3e04, 0x7810, 0x449a, 0x0040, 0x0000, 0x0803, 0x2800, 0x7c84, /*  330 */
  0x30f0, 0x4024, 0x0fff, 0xfec2, 0xa020, 0x0024, 0x0fff, 0xff02, /*  338 */
  0xa122, 0x0024, 0x4036, 0x0024, 0x0000, 0x1fc2, 0xb326, 0x0024, /*  340 */
  0x0010, 0x4002, 0x4326, 0x4495, 0x4024, 0x40d2, 0x0000, 0x0180, /*  348 */
  0xa100, 0x4090, 0x0010, 0x0042, 0x4204, 0x0024, 0xbc82, 0x4091, /*  350 */
  0x459a, 0x0024, 0x0000, 0x0054, 0x2800, 0x7b84, 0xbd86, 0x4093, /*  358 */
  0x2400, 0x7b45, 0xfe01, 0x5e0c, 0x5c43, 0x5f2d, 0x5e46, 0x0024, /*  360 */
  0x5c56, 0x0024, 0x5e53, 0x5e0c, 0x5c43, 0x5f2d, 0x5e46, 0x0024, /*  368 */
  0x5c56, 0x0024, 0x5e52, 0x0024, 0x4cb2, 0x4405, 0x0010, 0x4004, /*  370 */
  0x654a, 0x9810, 0x0000, 0x0144, 0xa54a, 0x1bd1, 0x0006, 0x0013, /*  378 */
  0x3301, 0xc444, 0x687e, 0x2005, 0xad76, 0x8445, 0x4ed6, 0x8784, /*  380 */
  0x36f3, 0x64c2, 0xac72, 0x8785, 0x4ec2, 0xa443, 0x3009, 0x2440, /*  388 */
  0x3009, 0x2741, 0x36f3, 0xd80e, 0x36f1, 0xd813, 0x36f1, 0x5806, /*  390 */
  0x36f0, 0x9803, 0x36f0, 0x1801, 0x2000, 0x0000, 0x36f2, 0x9814, /*  398 */
  0x0007, 0x0001, 0x8208, 0x0006, 0x000e, 0x4c82, 0x0024, 0x0000, /*  3a0 */
  0x0024, 0x2000, 0x0005, 0xf5c2, 0x0024, 0x0000, 0x0980, 0x2000, /*  3a8 */
  0x0000, 0x6010, 0x0024, 0x000a, 0x0001, 0x0050
};

//routine used to enable any plugin
//example 1: VS1053b's midi mode (rtmplugin); 
//example 2: VS1053b's spectrum analyzer (saplugin)
// size can be determined by using something like sizeof(chosenPlugin)/sizeof(chosenPlugin[0]), 
//    just select one of the plugin labels above and replace 'chosenPlugin'
void initVS1053Plugin(const uint16_t plugin[], uint16_t size) {
    uint16_t n;
    uint16_t addr, val, i=0;

  while (i<size) {
	  

    addr = plugin[i++];
    n = plugin[i++];
	
    if (n & 0x8000U) { /* RLE run, replicate n samples */
      n &= 0x7FFF;
      val = plugin[i++];
      while (n--) {
        //WriteVS10xxRegister(addr, val);
        POKE(VS_SCI_ADDR,addr);
        POKEW(VS_SCI_DATA,val);
        POKE(VS_SCI_CTRL,CTRL_Start);
        POKE(VS_SCI_CTRL,0);
		while ((PEEK(VS_SCI_CTRL) & CTRL_Busy) == CTRL_Busy)
			;
      }
    } else {           /* Copy run, copy n samples */
      while (n--) {
        val = plugin[i++];
        //WriteVS10xxRegister(addr, val);
        POKE(VS_SCI_ADDR,addr);
        POKEW(VS_SCI_DATA,val);
        POKE(VS_SCI_CTRL,CTRL_Start);
        POKE(VS_SCI_CTRL,0);
		while ((PEEK(VS_SCI_CTRL) & CTRL_Busy) == CTRL_Busy)
			;
      }
    }
  }
}

//perform this routine to boost the chip's clock. Pretty much necessary to do before real time midi mode or mp3 playback
void boostVSClock()
{
//target the clock register
POKE(VS_SCI_ADDR, VS_SCI_ADDR_CLOCKF);
//aim for 2.5X clock multiplier, no frills
POKE(VS_SCI_DATA,0x00);
POKE(VS_SCI_DATA+1,0xc0);
//trigger the command
POKE(VS_SCI_CTRL,CTRL_Start);
POKE(VS_SCI_CTRL,0);
//check to see if it's done
	while (PEEK(VS_SCI_CTRL) & CTRL_Busy)
		;
}

uint16_t checkClock()
{
POKE(VS_SCI_ADDR, VS_SCI_ADDR_CLOCKF);
//trigger the command
POKE(VS_SCI_CTRL, CTRL_Start | CTRL_RWn);
POKE(VS_SCI_CTRL,0);
//check to see if it's done
	while (PEEK(VS_SCI_CTRL) & CTRL_Busy)
		;
return PEEKW(VS_SCI_DATA);
}

uint16_t getNbBands()
{
	//Getting the number of bands (should be 14)
	//target the wram addr register
	POKE(VS_SCI_ADDR, VS_SCI_ADDR_WRAMADDR);
	//target the number of used bands in 0x1802
	POKEW(VS_SCI_DATA, 0x1802);
	//trigger the command
	POKE(VS_SCI_CTRL, CTRL_Start);
	POKE(VS_SCI_CTRL,0);
	//check to see if it's done
		while (PEEK(VS_SCI_CTRL) & CTRL_Busy)
			;
	//target the wram register
	POKE(VS_SCI_ADDR, VS_SCI_ADDR_WRAM);
	//trigger the read command
	POKE(VS_SCI_CTRL, CTRL_Start | CTRL_RWn);
	POKE(VS_SCI_CTRL,0);
	//check to see if it's done
		while (PEEK(VS_SCI_CTRL) & CTRL_Busy)
			;		
	return PEEKW(VS_SCI_DATA);
}
//Assuming the spectrum analyzer plugin has been loaded beforehand
//this can read the results for each of the default 14 bands
void getCenterSAValues(uint16_t nbBands, uint16_t *values)
{
uint16_t i = 0;
uint16_t confirmedBands = nbBands;

if(nbBands==0) confirmedBands = getNbBands();

//target the wram addr register
POKE(VS_SCI_ADDR, VS_SCI_ADDR_WRAMADDR);
//target the WRAM location that contains the values in 0x1804
POKEW(VS_SCI_DATA, 0x1804);
//trigger the write command
POKE(VS_SCI_CTRL, CTRL_Start);
POKE(VS_SCI_CTRL,0);
//check to see if it's done
		while (PEEK(VS_SCI_CTRL) & CTRL_Busy)
		;
	
for(i=0; i<confirmedBands; i++)
	{
//target the wram register
POKE(VS_SCI_ADDR, VS_SCI_ADDR_WRAM);
//trigger the read command
POKE(VS_SCI_CTRL, CTRL_Start | CTRL_RWn);
POKE(VS_SCI_CTRL,0);
//check to see if it's done
	while (PEEK(VS_SCI_CTRL) & CTRL_Busy)
		;	
	values[i] = (0x003F&(PEEKW(VS_SCI_DATA))); //only pick the values, discard the peaks
	}
}


//Assuming the spectrum analyzer plugin has been loaded beforehand
//this can read the center frequency for each of the default 14 bands
void getCenterSABands()
{
}
//Enable the Spectrum Analyzer
void initSpectrum()
{
const uint16_t *ptr = saplugin;
initVS1053Plugin(ptr, 950);
}

//Enable the real time MIDI mode
void initRTMIDI()
{
const uint16_t *ptr = rtmplugin;
initVS1053Plugin(ptr, 28);
}
}