MEMORY
{
  ram : ORIGIN = 0x10000, LENGTH = 0x10000
  rom : ORIGIN = 0x20000, LENGTH = 0x10000
}

SECTIONS
{
  .text : {*(.text)} > ram AT> rom
  .data : ALIGN (16) {*(.data)} > ram AT> rom
  /DISCARD/ : {*(*)}
}
