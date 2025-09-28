SECTIONS
{
  .text : {*(.text .pr)}
  . = ALIGN(data_align);
  .data : {*(.data .rw)}
  .bss : {*(.bss)}
  /DISCARD/ : {*(*)}
}
