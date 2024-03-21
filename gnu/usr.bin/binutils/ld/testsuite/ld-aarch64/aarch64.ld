/* Script for ld testsuite */
OUTPUT_ARCH(aarch64)
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
  } =0
  . = 0x9000;
  .got            : { *(.got) *(.got.plt)}
  . = 0x12340000;
  .far : { *(.far) }
  .ARM.attributes 0 : { *(.ARM.atttributes) }
}
