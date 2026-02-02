#define F256LIB_IMPLEMENTATION
#include "f256lib.h" 
// #include "..\src\muUtils.h" 

void FAR8_myFunc(void);

#pragma clang optimize off
__attribute__((noinline))
void myFunc(void) {
    volatile unsigned char ___mmu = (unsigned char)*(volatile unsigned char *)0x000d;
    *(volatile unsigned char *)0x000d = 8;
    FAR8_myFunc();
    *(volatile unsigned char *)0x000d = ___mmu;
}
__attribute__((noinline))
void myFunc2(void) {
    volatile unsigned char ___mmu = (unsigned char)*(volatile unsigned char *)0x000d;
    *(volatile unsigned char *)0x000d = 8;
    FAR8_myFunc2();
    *(volatile unsigned char *)0x000d = ___mmu;
}
#pragma clang optimize on

__attribute__((noinline, section(".block8")))
void FAR8_myFunc(void) {
printf("\nallo");
printf("allo");
printf("allo");
printf("allo");

}

__attribute__((noinline, section(".block8")))
void FAR8_myFunc2(void) {
printf("\nbonjour");
printf("bonjour");
printf("bonjour");
printf("bonjour");
printf("bonjour");

}
int main(int argc, char *argv[]) {    
    
    printf("start\n");
    myFunc();
    myFunc2();
    getchar();
    return 0;
}
}