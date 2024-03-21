SECTIONS
{
  . = 0x800000;
  .text : { *(.text) }
  . = 0x900000;
  .data : { *(.data) }
  /DISCARD/ : { *(.*) }
}
