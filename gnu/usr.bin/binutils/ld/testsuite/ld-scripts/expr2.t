SECTIONS
{
  . = 0xc000;
  .text :
  {
    _text = .;
    *(.text)
  }
  _end = .;
}
ASSERT (_end - _text <= 0x100, "fail");
