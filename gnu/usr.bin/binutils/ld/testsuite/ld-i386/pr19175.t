EXTERN(_start)
ENTRY(_start)

SECTIONS
{
  .text :
  {
    _text = .;
    *(.text*)
  }
}
