SECTIONS
{
  .text : { *(.text) }
  .data : { *(.data .rw) }
  .bss  : { *(.bss) }
  /DISCARD/ : { *(*) }
}
