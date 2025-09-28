SECTIONS
{
  . = 0x80;
  .data : { *(.data*) }
  . = 0x10000000;
  .text : { *(.text*) }
  /DISCARD/ : { *(*) }
}
