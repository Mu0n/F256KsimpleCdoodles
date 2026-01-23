#define F256LIB_IMPLEMENTATION
#include "f256lib.h"

void xsprite32(byte sprt, byte img, uint16_t x, uint16_t y);
void xsprite16(byte sprt, byte img, uint16_t x, uint16_t y);
void initLevel();
void drawtie();

uint16_t base;
uint32_t address;

EMBED(embeddedCLUT,"../gfx/master07.pal", 0x20000);
EMBED(embedded16,  "../gfx/gfx16.bin",    0x30000);
EMBED(embedded32,  "../gfx/gfx32.bin",    0x40000);

int main(int argc, char *argv[]) {

    printf("main.c test\n");
    initLevel();
    xsprite32(0,0,50,50);
    xsprite32(1,1,100,100);
    xsprite32(2,2,150,150);
    xsprite32(3,3,200,200);
while(true);
    return 0;
    } 

void initLevel(void) {
    POKE(0xD000,63);        // 1(txt)+2(ovrly)+4(grphc)+8(bmp)+16(tiles)+32(sprts)
    POKE(0xD001,0);            // 
    POKE(0xD002,64+16+4);     // Layer 0/1
                            // TM0 to Layer 0 (bin100=4) &
                            // TM1 to Layer 1 (bin101=64+16)
    POKE(0xD003,0);            // Layer 2  - BM0 to Layer 2

    POKE(0x01,1);            // transfer CLUT from 0x20000 to 0xD000
    for (int i = 0; i < 1023; i++) {
        POKE(0xD000+i,FAR_PEEK(0x20000+i));
        }
    POKE(0x01,0);
return;
}

void drawtie(void) {
    xsprite16(0,0,100,100);            // draw one Tie Sprite
    xsprite32(0,0,200,200);        // draw one Falcon Sprite
    xsprite32(0,5,300,300);        // draw one Falcon Sprite
    return;
}

void xsprite32(byte sprite, byte image, uint16_t x, uint16_t y) {
    base    = 0xD900  + (sprite * 8);
    printf("sprite no. %02X  ",sprite);
    printf("image no. %02X  ",image);
    printf("base %u  ",base);
    address = 0x40004 + ( image * 1026);
    printf("address %lu  \n",address);
    POKE(base,1);                         //32x32, CLUT0
    POKEA(base+1,address);                 //sprite address as long var 3 bytes endian L,M,H
    POKEW(base+4, x);
    POKEW(base+6, y);
    return;
}

void xsprite16(byte sprite, byte image, uint16_t x, uint16_t y) {
    base    = 0xD900  + (sprite * 8);
    address = 0x30004 + ( image * 258);
    POKE(base,65);                         //16x16, CLUT0
    POKEA(base+1,address);                 //sprite address as long var 3 bytes endian L,M,H
    POKEW(base+4, x);
    POKEW(base+6, y);
    return;
}

}