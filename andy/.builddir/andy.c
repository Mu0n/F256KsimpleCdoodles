#include "D:\F256\llvm-mos\code\andy\.builddir\trampoline.h"


#define F256LIB_IMPLEMENTATION

#include "f256lib.h"
 
 
int main(int argc, char *argv[]) {
    uint16_t  x;
    uint16_t  y;
    uint16_t  width;
    uint16_t  height;
    uint16_t  mx=320;
    uint16_t  my=240;
    byte      l;
    byte      c = 0;
 
    //F256LIB_RESET
    textReset();
    graphicsReset();
    bitmapReset();
    randomReset();
 
    for (l=0; l<TEXTCOLORS_COUNT; l++) {
        graphicsDefineColor(0, l, textColors[l].r, textColors[l].g, textColors[l].b);
    }
 
    bitmapSetColor(0);
    bitmapClear();
    bitmapSetVisible(0, true);
   // bitmapGetResolution(&mx, &my);
 
    bitmapSetColor(WHITE);
    //bitmapRectangle(1, 1, 318, 238);
    getchar();
 
    bitmapLine(0, 0,   319, 239);
    bitmapLine(0, 239, 319, 0);
    getchar();
 
    while (1) {
        bitmapSetColor(c++);
 
        if (c == TEXTCOLORS_COUNT)
            c = 0;
 
        x  = randomRead() % mx;
        y  = randomRead() % my;
        width  = randomRead() % mx;
        height = randomRead() % my;
 
        if ((x + width - 1) > mx)
            width = mx - x - 1;
        if ((y + height - 1) > my)
            width = my - y - 1;
 
        //bitmapLine(x, y, x2, y2);
        //bitmapRectangle(x, y, width, height);
    }
 
    return 0;
}
