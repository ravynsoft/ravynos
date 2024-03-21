SECTIONS
{
  . = SIZEOF_HEADERS;
  . = 0x800000;
  PROVIDE (bar = .);
  .data : {
    *(.data)
  }
}
