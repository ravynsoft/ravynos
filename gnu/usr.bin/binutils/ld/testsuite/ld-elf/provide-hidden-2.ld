SECTIONS
{
  . = 0x12300000;
  PROVIDE_HIDDEN (foo = ABSOLUTE (.) + 0x11100000);
  .data : { *(.data) }
  .got : { *(.got) }
  .interp : { *(.interp) }
  .dynsym : { *(.dynsym) }
  .dynstr : { *(.dynstr) }
  .dynamic : { *(.dynamic) }
  .hash : { *(.hash) }
  .gnu.hash : { *(.gnu.hash) }
}
