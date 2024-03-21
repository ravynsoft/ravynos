SECTIONS
{
  .text 0x0000000: { *(.text .pr) }
  .data :
  {
    PROVIDE (__data_start = .);
    *(.data)
  }
  __data_end = .;
  .bss : { *(.bss) }
  /DISCARD/ : { *(.*) }
}
