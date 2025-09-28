SECTIONS
{
  . = 0x800;
  A = .;
  .text :
  {
    _start = .;
    *(.text)
    . = 0x100;
  }
  B = .;
  .data :
  {
    *(.data)
    . = 0x100;
  }
  C = .;
  .bss :
  {
    *(.bss)
    . = 0x100;
  }
  D = A - C + B;
  E = A + B - C;
  /DISCARD/ : {*(*)}
}

ASSERT(D == E, "Addition is not commutative");
