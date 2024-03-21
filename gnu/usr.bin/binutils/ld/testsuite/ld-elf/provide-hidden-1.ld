SECTIONS
{
  . = SIZEOF_HEADERS;
  . = 0x12300000;
  .data :
    {
      PROVIDE_HIDDEN (foo = . + 0x11100000);
      *(.data)
    }
  .got : { *(.got) }
  .interp : { *(.interp) }
  .dynsym : { *(.dynsym) }
  .dynstr : { *(.dynstr) }
  .dynamic : { *(.dynamic) }
  .hash : { *(.hash) }
  .gnu.hash : { *(.gnu.hash) }
}
