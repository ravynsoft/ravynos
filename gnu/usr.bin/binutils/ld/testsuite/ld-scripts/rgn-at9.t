MEMORY
{
  ram : ORIGIN = 0x10000, LENGTH = 0x10000
  rom : ORIGIN = 0x20080, LENGTH = 0x10000
}

SECTIONS
{
  .text : ALIGN_WITH_INPUT {*(.text)} > ram AT> rom
  .data : ALIGN_WITH_INPUT {*(.data)} > ram AT> rom
  /DISCARD/ : {*(*)}
}
