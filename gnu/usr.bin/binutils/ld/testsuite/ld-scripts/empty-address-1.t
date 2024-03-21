SECTIONS
{
  .text 0: { *(.text .pr) }
  .data 0x200:
  {
    __data_start = . ;
    *(.data)
  }
  __data_end = .;
  .bss : { *(.bss) }
  /DISCARD/ : { *(.*) }
}
