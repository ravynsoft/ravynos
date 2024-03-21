SECTIONS
{
  .data 0x1000 :
  {
    *(.data .rw)
    QUAD (__FOO);
  }

  .foo 0x4000 :
  {
    PROVIDE (__FOO = .);
    *(.foo)
  }
}
