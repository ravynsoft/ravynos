NOCROSSREFS ( .text .data )
x = ABSOLUTE(x);
SECTIONS
{ 
  .text : { *(.text .pr) }
  .data : { *(.data .rw) }
  .bss : { *(.bss) }
  /DISCARD/ : { *(*) }
}
