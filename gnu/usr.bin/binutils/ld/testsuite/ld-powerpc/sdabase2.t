SECTIONS
{
  .text 0x100 : { *(.text) }
  .sdata 0x400 : { PROVIDE (_SDA_BASE_ = .); *(.sdata) }
  /DISCARD/ : { *(*) }
}
