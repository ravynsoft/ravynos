SECTIONS
{
  . = -0x7ff00000;
  .text : {*(.text .text.*)}
  . = ALIGN(64);
  .foo : { *(.foo) }
  . = ALIGN(8192);
  .data : AT (ADDR(.data)) { *(.data) }
  /DISCARD/ : { *(.*) }
}
