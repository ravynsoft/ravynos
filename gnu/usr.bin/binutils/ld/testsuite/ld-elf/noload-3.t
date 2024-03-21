SECTIONS
{
  .foo2 : { *(.foo2) }
  .foo1 : { *(.foo1) }
  .foo (NOLOAD) : { *(.foo) }
  /DISCARD/ : { *(*) }
}
