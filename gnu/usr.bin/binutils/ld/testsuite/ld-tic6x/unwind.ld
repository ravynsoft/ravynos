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
  . = 0x9000;
  .c6xabi.exidx : { *(.c6xabi.exidx*) }
  . = 0xa000;
  .got            : { *(.got) *(.got.plt)}
  . = 0x12340000;
  .far : { *(.far) }
  .c6xabi.attribues 0 : { *(.c6xabi.atttributes) }
}
