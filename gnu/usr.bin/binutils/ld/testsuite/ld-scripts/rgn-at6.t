MEMORY
{
  ram : ORIGIN = 0x10000, LENGTH = 0x10000
}

SECTIONS
{
  .text : {*(.text)} > ram AT> ram
  .data : ALIGN (16) {*(.data)} > ram AT> ram
  /DISCARD/ : {*(*)}
}
