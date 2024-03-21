SECTIONS 
{
  . = SIZEOF_HEADERS;
  PROVIDE (foo = 1);
  PROVIDE (bar = 2);
  PROVIDE (baz = 3);
  .data 0x2000 :
  {
    *(.data .rw)
  }
}
