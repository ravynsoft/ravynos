/* Script for unwinding ld tests */
SECTIONS
{
  /* Read-only sections, merged into text segment: */
  . = 0x8000;
  .text           :
  {
    *(.before)
    *(.text)
    *(.after)
    *(.c6xabi.extab*)
  } =0
  /DISCARD/ : { *(.c6xabi.exidx*) }
  .c6xabi.attribues 0 : { *(.c6xabi.atttributes) }
}
