SECTIONS
{
  def1 = DEFINED(foo) ? 0x10 : 0x20;
  def2 = def1;
  . = 0x10001;
  foo = .;
  . += 0x200;
  bar = .;
  . = ALIGN (4);
  frob = .;

  . = 0x10000;
  .text : { *(.text) }
}
