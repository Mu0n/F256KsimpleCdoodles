#include "D:\F256\llvm-mos\code\float\.builddir\trampoline.h"

#include "f256lib.h"
#include <stdint.h>
#include "..\src\fp_module.h"
#include <stdio.h>
#include <string.h>

volatile uint8_t g_fp_last_status = 0;

// TODO:  FIXED POINT ISN"T PROPERLY IMPLEMENTED YET
fixed19_12_t mathFixedAdd(fixed19_12_t a, fixed19_12_t b)
{
    fixed19_12_t result=0;
    // Control 1
    POKE(0xDE80, 0x43); // fixed point inputs for addition
    POKE(0xDE81, 0x02); // adder output
    memcpy((char*) 0xDE88, &a, sizeof(fixed19_12_t)); // input 0
    memcpy((char*) 0xDE8C, &b, sizeof(fixed19_12_t)); // input 1
    POKE(0xDE82, 0x05); // input0 and input1 valid converter input
    // printf("Starting fixed point addition...\n");
    while ((PEEK(0xDE86) & 0x10) == 0); // wait for add result valid
    // printf("Addition complete.\n");
    memcpy(&result, (char*) 0xDE88, sizeof(fixed19_12_t));
    // printf("Add Result: %lX\n", result);
    while ((PEEK(0xDE87) & 0x08) == 0); // wait for convert result valid
    // printf("Conversion complete.\n");
    memcpy(&result, (char*) 0xDE8C, sizeof(fixed19_12_t));

    return result;

}
fixed19_12_t mathFixedSub(fixed19_12_t a, fixed19_12_t b){
    // assembly here
    return 0;
}

fixed19_12_t mathFixedMul(fixed19_12_t a, fixed19_12_t b) // Returns (a*b) >> 12
{
    // assembly here
    return 0;
}
fixed19_12_t mathFixedDiv(fixed19_12_t a, fixed19_12_t b) // Returns (a<<12) / b
{
    // assembly here
    return 0;
}


// --- IEEE 754 Floating Point Math ---

float mathFloatAdd(float a, float b);
// a: A,X,__rc2, __rc3
// b: __rc4, __rc5, __rc6, __rc7
// result: A,X,__rc2,__rc3
    asm (
        ".text\n"
        ".global mathFloatAdd\n"
        "mathFloatAdd:\n"
        "sta $de88\n"
        "stx $de89\n"
        "lda #$40\n"
        "sta $de80\n"      // control 0
        "lda #$02\n"
        "sta $de81\n"      // control 1 - float point add/sub
        "lda __rc2\n"
        "sta $de8a\n"
        "lda __rc3\n"
        "sta $de8b\n"
        "lda __rc4\n"
        "sta $de8c\n"
        "lda __rc5\n"
        "sta $de8d\n"
        "lda __rc6\n"
        "sta $de8e\n"
        "lda __rc7\n"
        "sta $de8f\n"
        "lda #$0a\n"
        "sta $de82\n"      // control 2 - input0 and input1 valid
        "nop\n"            // 6 clock latency @ 25MHz
        "nop\n"
        "nop\n"
        "nop\n"
        "nop\n"
        "lda $de86\n"   // status register
        "and #$0F\n"    // mask out output valid bit
        "sta g_fp_last_status\n"
        "lda $de8a\n"
        "sta __rc2\n"
        "lda $de8b\n"
        "sta __rc3\n"
        "lda $de88\n"
        "ldx $de89\n"
        "rts\n"
    );


float mathFloatSub(float a, float b);
// a: A,X,__rc2, __rc3
// b: __rc4, __rc5, __rc6, __rc7
// result: A,X,__rc2,__rc3
    asm (
        ".text\n"
        ".global mathFloatSub\n"
        "mathFloatSub:\n"
        "sta $de88\n"
        "stx $de89\n"
        "lda #$48\n"
        "sta $de80\n"      // control 0
        "lda #$02\n"
        "sta $de81\n"      // control 1 - float point add/sub
        "lda __rc2\n"
        "sta $de8a\n"
        "lda __rc3\n"
        "sta $de8b\n"
        "lda __rc4\n"
        "sta $de8c\n"
        "lda __rc5\n"
        "sta $de8d\n"
        "lda __rc6\n"
        "sta $de8e\n"
        "lda __rc7\n"
        "sta $de8f\n"
        "lda #$0a\n"
        "sta $de82\n"      // control 2 - input0 and input1 valid
        "nop\n"            // 6 clock latency @ 25MHz
        "nop\n"
        "nop\n"
        "nop\n"
        "nop\n"
        "lda $de86\n"   // status register
        "and #$0F\n"    // mask out output valid bit
        "sta g_fp_last_status\n"
        "lda $de8a\n"
        "sta __rc2\n"
        "lda $de8b\n"
        "sta __rc3\n"
        "lda $de88\n"
        "ldx $de89\n"
        "rts\n"
    );

float mathFloatMul(float a, float b);
// a: A,X,__rc2, __rc3
// b: __rc4, __rc5, __rc6, __rc7
// result: A,X,__rc2,__rc3
    asm (
        ".text\n"
        ".global mathFloatMul\n"
        "mathFloatMul:\n"
        "sta $de88\n"
        "stx $de89\n"
        "lda #$00\n"
        "sta $de80\n"      // control 0
        "sta $de81\n"      // control 1 - float point multiply
        "lda __rc2\n"
        "sta $de8a\n"
        "lda __rc3\n"
        "sta $de8b\n"
        "lda __rc4\n"
        "sta $de8c\n"
        "lda __rc5\n"
        "sta $de8d\n"
        "lda __rc6\n"
        "sta $de8e\n"
        "lda __rc7\n"
        "sta $de8f\n"
        "lda #$0a\n"
        "sta $de82\n"      // control 2 - input0 and input1 valid
        "nop\n"            // 6 clock latency @ 25MHz
        "nop\n"
        "nop\n"
        "nop\n"
        "nop\n"
        "lda $de84\n"   // status register
        "and #$0F\n"    // mask out output valid bit
        "sta g_fp_last_status\n"
        "lda $de8a\n"
        "sta __rc2\n"
        "lda $de8b\n"
        "sta __rc3\n"
        "lda $de88\n"
        "ldx $de89\n"
        "rts\n"
    );

float mathFloatDiv(float a, float b);
// a: A,X,__rc2, __rc3
// b: __rc4, __rc5, __rc6, __rc7
// result: A,X,__rc2,__rc3
    asm (
        ".text\n"
        ".global mathFloatDiv\n"
        "mathFloatDiv:\n"
        "sta $de88\n"
        "stx $de89\n"
        "lda #$00\n"
        "sta $de80\n"      // control 0
        "lda #$01\n"
        "sta $de81\n"      // control 1 - float point divide
        "lda __rc2\n"
        "sta $de8a\n"
        "lda __rc3\n"
        "sta $de8b\n"
        "lda __rc4\n"
        "sta $de8c\n"
        "lda __rc5\n"
        "sta $de8d\n"
        "lda __rc6\n"
        "sta $de8e\n"
        "lda __rc7\n"
        "sta $de8f\n"
        "lda #$0a\n"
        "sta $de82\n"      // control 2 - input0 and input1 valid
        "nop\n"            // 14 clock latency @ 25MHz
        "nop\n"
        "nop\n"
        "nop\n"
        "nop\n"
        "nop\n"
        "nop\n"
        "nop\n"
        "lda $de85\n"   // status register
        "and #$1F\n"    // mask out output valid bit
        "sta g_fp_last_status\n"
        "lda $de8a\n"
        "sta __rc2\n"
        "lda $de8b\n"
        "sta __rc3\n"
        "lda $de88\n"
        "ldx $de89\n"
        "rts\n"
    );
