_START = DEFINED(_START) ? _START : 0x900;
SECTIONS
{
  . = _START;
  .text : {*(.text .pr)}
  .data : {*(.data)}
  .bss : {*(.bss)}
  /DISCARD/ : {*(*)}
}
