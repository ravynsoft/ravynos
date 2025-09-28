SECTIONS
{
  .data 0x1000 :
  {
    *(.data .rw)
  }

  foo = ADDR (.data);
  foo_2 = foo;
  PROVIDE (foo = 0x20);
}
