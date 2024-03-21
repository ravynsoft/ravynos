EXTERN(_start)
ENTRY(_start)

SECTIONS
{
  .text :
  {
    HIDDEN (__hidden_sym = .);
    *(.text*)
  }
}
