SECTIONS
{
  .text 0x100 : { *(.text) }
  .sdata 0x400 : { *(.sdata) }
  /DISCARD/ : { *(*) }
}
