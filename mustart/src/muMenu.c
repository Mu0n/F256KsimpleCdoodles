#include "f256lib.h"
#include "../src/muMenu.h"

char menuItems[10][120];

uint8_t itemCount = 5;

int readLine(FILE *fp, char *buf, int max) {
    int i = 0;
    char c;

    while (fileRead(&c, 1, 1, fp) == 1) {
		if (c == 0x0a) //line feed control detected
            break;
			
        if (c == 0x0d) //carriage return detected
            break;

        if (i < max - 1)
            buf[i++] = c;
    }

    if (i == 0 && c != 0x0d)
        return 0; // EOF

    buf[i] = '\0';
    return 1;
}

FILE *loadMenuFile(void)
{
	FILE *theVGMfile;
	theVGMfile = fileOpen("vcfmenu.txt","r"); // open file in read mode
	if(theVGMfile == NULL) {
		return NULL;
		}
	return theVGMfile;
}
void displayMenu(uint8_t x, uint8_t y)
{
	FILE *theFile = loadMenuFile();
	
	for(uint8_t i=0; i < itemCount; i++)
	{
	readLine(theFile, menuItems[i], 120);
	displayOneItem(x,y,i);
	}
}
void displayOneItem(uint8_t x, uint8_t y, uint8_t which)
{
	textGotoXY(x,y+which);
	printf("%s",menuItems[which]);
}
}