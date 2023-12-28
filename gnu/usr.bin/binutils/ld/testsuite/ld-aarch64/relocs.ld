/* Script for ld testsuite */
OUTPUT_ARCH(aarch64)
ENTRY(_start)
SECTIONS
{
  /* Read-only sections, merged into text segment: */
  PROVIDE (__executable_start = 0x8000); . = 0x10000;
  .text           :
  {
    *(.before)
    *(.text)
    *(.after)
  } =0
  . = 0x20000;
  .got            : { *(.got) *(.got.plt)}
  . = 0x12340000;
  .far : { *(.far) }
  .ARM.attributes 0 : { *(.ARM.atttributes) }
  /* Providing a .data section triggers a non-executable segment to be
     generated, which gives somewhere else for the linker to put the TLS
     segment without creating a RWX permissioned one. */
  .data : { *(.data) }
}
