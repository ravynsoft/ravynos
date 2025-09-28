OUTPUT_FORMAT("elf32-i386")
OUTPUT_ARCH(i386)
ENTRY(_start)
SECTIONS
{
  .text : {*(.text)}
  .data : {*(.data)}
  /DISCARD/ : {*(*)}
}
