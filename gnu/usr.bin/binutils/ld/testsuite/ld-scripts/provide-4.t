SECTIONS
{
  . = SIZEOF_HEADERS;
  PROVIDE (foo = 1);
  PROVIDE (bar = 2);
  PROVIDE (baz = 3);
  .data 0x2000 :
  {
    *(.data .rw)

    PROVIDE (loc1 = ALIGN (., 0x10));
    PROVIDE (loc2 = ALIGN (., 0x10));
  }

  PROVIDE (loc3 = loc1 + 0x20);
  loc4 = loc2 + 0x20;
}
