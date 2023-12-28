SECTIONS
{
  . = 0x2000 ;

  _start = .;

  HEAP_SIZE = 0x100;

  .heap : {
    . = HEAP_SIZE;
    . = ALIGN(4);
  }

  _end = .;

  .text : { *(.text) }
  .data : { *(.data) }
  .bss : { *(.bss) }
  /DISCARD/ : { *(*) }
}
