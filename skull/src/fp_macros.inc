// macros clobber A and status

.macro fp_add arg_a, arg_b, result
        lda #$40
        sta $de80      // control 0
        lda #$02
        sta $de81      // control 1 - float point add/sub
		lda \arg_a
		sta $de88
		lda \arg_a+1
		sta $de89
		lda \arg_a+2
		sta $de8a
		lda \arg_a+3
		sta $de8b
		lda \arg_b
		sta $de8c
		lda \arg_b+1
		sta $de8d
		lda \arg_b+2
		sta $de8e
		lda \arg_b+3
		sta $de8f
		lda #$0a
		sta $de82      // control 2 - input0 and input1 valid
		nop              // 6 clock latency @ 25MHz 
		nop
		nop
		nop
		nop
		lda $de86     // status register
		and #$0F      // mask out output valid bit
		sta g_fp_last_status
		lda $de88
		sta \result
		lda $de89
		sta \result+1
		lda $de8a
		sta \result+2
		lda $de8b
		sta \result+3
.endm

.macro fp_sub arg_a, arg_b, result
        lda #$48
        sta $de80      // control 0
        lda #$02
        sta $de81      // control 1 - float point add/sub
		lda \arg_a
		sta $de88
		lda \arg_a+1
		sta $de89
		lda \arg_a+2
		sta $de8a
		lda \arg_a+3
		sta $de8b
		lda \arg_b
		sta $de8c
		lda \arg_b+1
		sta $de8d
		lda \arg_b+2
		sta $de8e
		lda \arg_b+3
		sta $de8f
		lda #$0a
		sta $de82      // control 2 - input0 and input1 valid
		nop              // 6 clock latency @ 25MHz 
		nop
		nop
		nop
		nop
		lda $de86     // status register
		and #$0F      // mask out output valid bit
		sta g_fp_last_status
		lda $de88
		sta \result
		lda $de89
		sta \result+1
		lda $de8a
		sta \result+2
		lda $de8b
		sta \result+3
.endm

.macro fp_mul arg_a, arg_b, result
        lda #$00
        sta $de80      // control 0
        sta $de81      // control 1 - float point mul
		lda \arg_a
		sta $de88
		lda \arg_a+1
		sta $de89
		lda \arg_a+2
		sta $de8a
		lda \arg_a+3
		sta $de8b
		lda \arg_b
		sta $de8c
		lda \arg_b+1
		sta $de8d
		lda \arg_b+2
		sta $de8e
		lda \arg_b+3
		sta $de8f
		lda #$0a
		sta $de82      // control 2 - input0 and input1 valid
		nop              // 6 clock latency @ 25MHz 
		nop
		nop
		nop
		nop
		lda $de84     // status register
		and #$0F      // mask out output valid bit
		sta g_fp_last_status
		lda $de88
		sta \result
		lda $de89
		sta \result+1
		lda $de8a
		sta \result+2
		lda $de8b
		sta \result+3
.endm

.macro fp_div arg_a, arg_b, result
        lda #$00
        sta $de80      // control 0
		lda #$01
        sta $de81      // control 1 - float point div
		lda \arg_a
		sta $de88
		lda \arg_a+1
		sta $de89
		lda \arg_a+2
		sta $de8a
		lda \arg_a+3
		sta $de8b
		lda \arg_b
		sta $de8c
		lda \arg_b+1
		sta $de8d
		lda \arg_b+2
		sta $de8e
		lda \arg_b+3
		sta $de8f
		lda #$0a
		sta $de82      // control 2 - input0 and input1 valid
		nop              // 14 clock latency @ 25MHz 
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		lda $de85     // status register
		and #$1F      // mask out output valid bit
		sta g_fp_last_status
		lda $de88
		sta \result
		lda $de89
		sta \result+1
		lda $de8a
		sta \result+2
		lda $de8b
		sta \result+3
.endm
