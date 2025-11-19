#include "D:\F256\llvm-mos\code\mid\.builddir\trampoline.h"

#define F256LIB_IMPLEMENTATION
#include "f256lib.h"

#include "../src/muUtils.h"


void print_byte_binary(unsigned char);

void print_byte_binary(unsigned char byte) {
    for (int i = 7; i >= 0; i--) {
        printf("%c", (byte & (1 << i)) ? '1' : '0');
    }
}



int main(int argc, char *argv[]) {

printf("Your machine ID is %02x in hex\n",PEEK(0xD6A7));
printf("or ");
print_byte_binary(PEEK(0xD6A7));
printf(" in binary\n\nhit space to exit");

hitspace();


}
