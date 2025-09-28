SECTIONS 
{
  .data :
  {
    LONG (foo)
    LONG (bar)
    *(.data .rw)
  }
  foo = .;
  bar = .;
}
