SECTIONS
{
  foo = 0x10;
  PROVIDE (foo = bar);

  .data 0x1000 :
  {
    *(.data .rw)
  }
}
