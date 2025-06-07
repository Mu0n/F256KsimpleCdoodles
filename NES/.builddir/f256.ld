/* fake C Stack */
PROVIDE(__stack = 0xC000);
/* entry point to my program - 0x200 to 0x2ff are command line arguments */
PROVIDE(__f256_start = 0x300);
/* page size of a block of memory */
PROVIDE(__BLOCK_SIZE = 0x2000);
/* swappable block address */
PROVIDE(__SLOT_ADDR = 0xC000);

/* f256k uses first 16 bytes of ZP for mmu control? */
__rc0 = 0x10;
INCLUDE imag-regs.ld
ASSERT(__rc0 == 0x10, "Inconsistent zero page map.")
ASSERT(__rc31 == 0x2f, "Inconsistent zero page map.")

MEMORY {
    /* kernel uses 0xf0-0xff for parameter passing */
    zp : ORIGIN = __rc31 + 1, LENGTH = 0xF0 - (__rc31 + 1)
    ram (rw) : ORIGIN = __f256_start, LENGTH = __stack-__f256_start
}

/* LMAs */
__block8_lma	= ( 8<<24)|__SLOT_ADDR;
__block9_lma	= ( 9<<24)|__SLOT_ADDR;
__block10_lma	= (10<<24)|__SLOT_ADDR;
__block11_lma	= (11<<24)|__SLOT_ADDR;
__block12_lma	= (12<<24)|__SLOT_ADDR;
__block13_lma	= (13<<24)|__SLOT_ADDR;
__block14_lma	= (14<<24)|__SLOT_ADDR;
__block15_lma	= (15<<24)|__SLOT_ADDR;
__block16_lma	= (16<<24)|__SLOT_ADDR;
__block17_lma	= (17<<24)|__SLOT_ADDR;
__block18_lma	= (18<<24)|__SLOT_ADDR;
__block19_lma	= (19<<24)|__SLOT_ADDR;
__block20_lma	= (20<<24)|__SLOT_ADDR;
__block21_lma	= (21<<24)|__SLOT_ADDR;
__block22_lma	= (22<<24)|__SLOT_ADDR;
__block23_lma	= (23<<24)|__SLOT_ADDR;

/* Stash preloaded binary data */
INCLUDE declare.ld

MEMORY {
  block8     : ORIGIN = __block8_lma,     LENGTH = __BLOCK_SIZE
  block9     : ORIGIN = __block9_lma,     LENGTH = __BLOCK_SIZE
  block10    : ORIGIN = __block10_lma,    LENGTH = __BLOCK_SIZE
  block11    : ORIGIN = __block11_lma,    LENGTH = __BLOCK_SIZE
  block12    : ORIGIN = __block12_lma,    LENGTH = __BLOCK_SIZE
  block13    : ORIGIN = __block13_lma,    LENGTH = __BLOCK_SIZE
  block14    : ORIGIN = __block14_lma,    LENGTH = __BLOCK_SIZE
  block15    : ORIGIN = __block15_lma,    LENGTH = __BLOCK_SIZE
  block16    : ORIGIN = __block16_lma,    LENGTH = __BLOCK_SIZE
  block17    : ORIGIN = __block17_lma,    LENGTH = __BLOCK_SIZE
  block18    : ORIGIN = __block18_lma,    LENGTH = __BLOCK_SIZE
  block19    : ORIGIN = __block19_lma,    LENGTH = __BLOCK_SIZE
  block20    : ORIGIN = __block20_lma,    LENGTH = __BLOCK_SIZE
  block21    : ORIGIN = __block21_lma,    LENGTH = __BLOCK_SIZE
  block22    : ORIGIN = __block22_lma,    LENGTH = __BLOCK_SIZE
  block23    : ORIGIN = __block23_lma,    LENGTH = __BLOCK_SIZE
  INCLUDE memory.ld
}

REGION_ALIAS("c_writeable", ram)
REGION_ALIAS("c_readonly", ram)

SECTIONS {
  INCLUDE c.ld
  .block8     : { *(.block8     .block8.*)      } >block8      end_block8 = .;
  .block9     : { *(.block9     .block9.*)      } >block9      end_block9 = .;
  .block10    : { *(.block10    .block10.*)     } >block10     end_block10 = .;
  .block11    : { *(.block11    .block11.*)     } >block11     end_block11 = .;
  .block12    : { *(.block12    .block12.*)     } >block12     end_block12 = .;
  .block13    : { *(.block13    .block13.*)     } >block13     end_block13 = .;
  .block14    : { *(.block14    .block14.*)     } >block14     end_block14 = .;
  .block15    : { *(.block15    .block15.*)     } >block15     end_block15 = .;
  .block16    : { *(.block16    .block16.*)     } >block16     end_block16 = .;
  .block17    : { *(.block17    .block17.*)     } >block17     end_block17 = .;
  .block18    : { *(.block18    .block18.*)     } >block18     end_block18 = .;
  .block19    : { *(.block19    .block19.*)     } >block19     end_block19 = .;
  .block20    : { *(.block20    .block20.*)     } >block20     end_block20 = .;
  .block21    : { *(.block21    .block21.*)     } >block21     end_block21 = .;
  .block22    : { *(.block22    .block22.*)     } >block22     end_block22 = .;
  .block23    : { *(.block23    .block23.*)     } >block23     end_block23 = .;
  INCLUDE sections.ld
}

OUTPUT_FORMAT {
  BYTE(0x5A)    /* pgZ */

  /* RAM Segment */
  SHORT(ORIGIN(ram))              /* where to load it, 24 bits */
  BYTE(0x00)
  SHORT(__bss_start-ORIGIN(ram))  /* size to load */
  BYTE(0x00)
  TRIM(ram)

  /* Overlay Segments and Binary Data */
  INCLUDE output.ld

  /* Launch the program, at _start */
  SHORT(_start)
  LONG(0)
}
