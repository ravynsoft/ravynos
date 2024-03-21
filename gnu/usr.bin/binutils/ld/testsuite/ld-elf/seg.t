
SECTIONS
{
  reset - 4 :
  {
    *(reset)
  }
  boot - 0x1000 :
  {
    *(boot)
  } = 0xffff
  . = + SIZEOF_HEADERS;
  .text : { *(.text) }
  .data : { *(.data) }
  .bss : { *(.bss) }
}
