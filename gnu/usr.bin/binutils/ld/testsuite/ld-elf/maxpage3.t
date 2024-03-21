SECTIONS
{
  . = SIZEOF_HEADERS;
  .text : {*(.text)}
  . = ALIGN(CONSTANT (MAXPAGESIZE));
  .data : {*(.data)}
  /DISCARD/ : {*(*)}
}
