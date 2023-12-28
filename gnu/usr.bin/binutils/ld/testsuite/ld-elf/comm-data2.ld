SECTIONS
{
  . = 0x12340000;
  .bss : { *(.dynbss) }
  .got : { *(.got.plt) *(.got) }
  .dynamic : { *(.dynamic) }
  .data : { *(.data) }
  . = 0x56780000;
  .rela.dyn : { *(.rela.*) }
  .interp : { *(.interp) }
  .hash : { *(.hash) }
  .gnu.hash : { *(.gnu.hash) }
  .dynsym : { *(.dynsym) }
  .dynstr : { *(.dynstr) }
  . = 0x76540000;
  .debug_foo : { *(.debug_foo) }
  .shstrtab : { *(.shstrtab) }
  .symtab : { *(.symtab) }
  .strtab : { *(.strtab) }
  /DISCARD/ : { *(*) }
}
