#ifndef MULEDS_H
#define MULEDS_H

#define SYS_CTRL_REG   0xD6A0  //register that can reset the machine, or enable manual control for LEDs

/* for register SYS_CTRL_REG, set 1 to assume manual control of these LEDs
   bit7: used for reset, not used here
   bit6: K2 only network LED
   bit5: K or K2 only caps lock LED
   bit4: 'buzz' (unused here)
   bit3: onboard LED1 (more visible on a Jr or JrJr)
   bit2: onboard LED0 (more visible on a Jr or JrJr)
   bit1: SD access LED
   bit0: power LED
*/
#define SYS_NET 0x40
#define SYS_LCK 0x20
#define SYS_L1  0x08
#define SYS_L0  0x04
#define SYS_SD  0x02
#define SYS_PWR 0x01


#define LED_PWR_B      0xD6A7  //power blue
#define LED_PWR_G      0xD6A8  //power green
#define LED_PWR_R      0xD6A9  //power red
#define LED_SD_B       0xD6AA  //media blue
#define LED_SD_G       0xD6AB  //media green
#define LED_SD_R       0xD6AC  //media red
#define LED_LCK_B      0xD6AD  //lock blue, K or K2 only
#define LED_LCK_G      0xD6AE  //lock green, K or K2 only 
#define LED_LCK_R      0xD6AF  //lock red, K or K2 only
#define LED_NET_B      0xD6B3  //network blue, K2 only
#define LED_NET_G      0xD6B4  //network green, K2 only
#define LED_NET_R      0xD6B5  //network red, K2 only

#include "f256lib.h"

void enableManualLEDs(bool, bool, bool, bool, bool, bool);

#endif // MULEDS_H