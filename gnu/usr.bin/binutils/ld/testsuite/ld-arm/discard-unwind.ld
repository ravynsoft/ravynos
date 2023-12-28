/* Script for ld testsuite */
OUTPUT_ARCH(arm)
ENTRY(_start)
SECTIONS
{
  /* Read-only sections, merged into text segment: */
  PROVIDE (__executable_start = 0x8000); . = 0x8000;
  .text           :
  {
    *(.before)
    *(.text)
    *(.after)
    *(.ARM.extab*)
    *(.glue_7)
    *(.v4_bx)
  } =0
  /DISCARD/ : { *(.ARM.exidx*) }
  .ARM.attribues 0 : { *(.ARM.atttributes) }
}
