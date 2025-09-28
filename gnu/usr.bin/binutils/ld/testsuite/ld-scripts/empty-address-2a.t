SECTIONS
{
  .text : { *(.text .pr) }
  .data : { *(.data) }
  __data_end = .;
  .bss : { *(.bss) }
  /DISCARD/ : { *(.*) }
}
