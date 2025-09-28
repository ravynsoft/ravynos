SECTIONS
{
  bar = 0x2345678;
  .text : { *(.text) }
  /DISCARD/ : { *(*) }
}
