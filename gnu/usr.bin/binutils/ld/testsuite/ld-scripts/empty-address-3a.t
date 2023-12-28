SECTIONS
{
  .text 0x00000000: { *(.text .pr) }
  .data ALIGN(0x1000) + (. & (0x1000 - 1)):
  {
    *(.data)
  }
  __data_end = .;
  .bss : { *(.bss) }
  /DISCARD/ : { *(.*) }
}
