#define F256LIB_IMPLEMENTATION
#define EIGHTK 0x2000

#include "f256lib.h"
#include "..\src\muUtils.h"
// Single-byte
#define MMU_MEM_CTRL_EXT_0 0x0002
#define MMU_MEM_CTRL_EXT_1 0x0003


uint8_t peek24(uint32_t addr);
uint8_t poke24(uint32_t addr, uint8_t value);


asm (
    ".text                  \n"
    ".global peek24         \n"
    "peek24:                \n"
    "   sta     $5          \n"
    "   stx     $6          \n"
    "   lda     __rc2       \n"
    "   sta     $7          \n"
    "   ldx     $0          \n"
    "   ldy     $1          \n"
    "   txa                 \n"
    "   ora     #$8         \n"
    "   php                 \n"
    "   sei                 \n"
    "   sta     $0          \n"
    "   tya                 \n"
    "   ora     #48         \n"
    "   sta     $1          \n"
    "   .byte   0xa7,0x05   \n"
    "   stx     $0          \n"
    "   sty     $1          \n"
    "   plp                 \n"
    "   ldx     #0          \n"
    "   rts                 \n"
);

asm (
    ".text                  \n"
    ".global poke24         \n"
    "poke24:                \n"
    "   sta     $5          \n"
    "   stx     $6          \n"
    "   lda     __rc2       \n"
    "   sta     $7          \n"
    "   ldx     $0          \n"
    "   ldy     $1          \n"
    "   txa                 \n"
    "   ora     #$8         \n"
    "   php                 \n"
    "   sei                 \n"
    "   sta     $0          \n"
    "   tya                 \n"
    "   ora     #$48         \n"
    "   sta     $1          \n"
    "   lda     __rc4       \n"
    "   .byte   0x87,0x05   \n"
    "   stx     $0          \n"
    "   sty     $1          \n"
    "   plp                 \n"
    "   ldx     #0          \n"
    "   rts                 \n"
);

int main(int argc, char *argv[]) {
short i =0;

char test1[]="hello";
char test2[]="2nd step";

printf("\n\n");
 

printf("writing at 0x17F FF0\n");
for(i=0;i<5;i++) printf("%c",poke24(0x17FFF0+i, test1[i]));
printf("\n");
printf("%02x%02x%02x",PEEK(7),PEEK(6),PEEK(5));
printf("\n");
for(i=0;i<5;i++) printf("%c",peek24(0x17FFF0+i));

printf("\n\n");
printf("writing at 0x180 000\n");
for(i=0;i<5;i++) printf("%c",poke24(0x180000+i, test1[i]));
printf("\n");
printf("%02x%02x%02x",PEEK(7),PEEK(6),PEEK(5));
printf("\n");
for(i=0;i<5;i++) printf("%c",peek24(0x180000+i));

printf("\n\n");
printf("writing at 0x1FF F00\n");
for(i=0;i<5;i++) printf("%c",poke24(0x1FFF00+i, test1[i]));
printf("\n");
printf("%02x%02x%02x",PEEK(7),PEEK(6),PEEK(5));
printf("\n");
for(i=0;i<5;i++) printf("%c",peek24(0x1FFF00+i));
hitspace();
printf("yes");

while(1)
	;

return 0;}



