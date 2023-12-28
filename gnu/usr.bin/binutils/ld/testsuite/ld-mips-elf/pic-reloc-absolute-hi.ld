SECTIONS
{
  bar = 0x12345678;
  .text : { *(.text) }
  /DISCARD/ : { *(*) }
}
