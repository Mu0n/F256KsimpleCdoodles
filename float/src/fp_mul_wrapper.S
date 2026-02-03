#include "fp_macros.inc"

    .global fp_mul_wrapper

; void fp_mul_wrapper(const uint8_t* a, const uint8_t* b, uint8_t* result);
fp_mul_wrapper:
    ; C calling convention (LLVM-MOS):
    ;   a = ptr in A/X
    ;   b = ptr in Y/Z
    ;   result = ptr on stack (first argument)

    ; Load arguments into zero-page temporaries
    sta arg_a
    stx arg_a+1
    sty arg_b
    stz arg_b+1

    ; Load result pointer from stack
    lda 2,s
    sta result
    lda 3,s
    sta result+1

    ; Call the macro
    fp_mul arg_a, arg_b, result

    rts

    .zeropage
arg_a:   .res 2
arg_b:   .res 2
result:  .res 2