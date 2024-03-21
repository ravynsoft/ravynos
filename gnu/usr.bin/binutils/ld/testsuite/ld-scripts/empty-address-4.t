SECTIONS
{
  .text 0: { *(.text .pr) }
  .data 0x200:
  {
    *(.data)
    ASSERT (. < 0x400, oops);
  }
  .bss : { *(.bss) }
  /DISCARD/ : { *(.*) }
}
