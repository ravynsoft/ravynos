/* Script for ld testsuite.  */
OUTPUT_ARCH(arm)
ENTRY(_start)
MEMORY
{
 read_memory   (rx)   : ORIGIN = 0x00008000, LENGTH = 4M
 purecode_memory (!rx)  : ORIGIN = 0x00800000, LENGTH = 4M
}
SECTIONS
{
  /* Read-only sections, merged into text segment: */
  PROVIDE (__executable_start = 0x8000); . = 0x8000;
  .text.purecode      :
    {
        INPUT_SECTION_FLAGS (SHF_ARM_PURECODE) *(.text*)
    } > purecode_memory
  .text           :
  {
    *(.before)
    *(.text)
    *(.after)
    *(.ARM.extab*)
    *(.glue_7)
    *(.v4_bx)
  } > read_memory
  .ARM.exidx : { *(.ARM.exidx*) }
  . = 0x9000;
  .got            : { *(.got) *(.got.plt)}
  . = 0x12340000;
  .far : { *(.far) }
  .ARM.attribues 0 : { *(.ARM.atttributes) }
}
