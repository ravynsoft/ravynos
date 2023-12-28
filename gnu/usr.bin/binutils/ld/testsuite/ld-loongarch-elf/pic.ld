SECTIONS
{
  .bss : { *(.dynbss) }
  .got : { *(.got.plt) *(.got) }
  .dynamic : { *(.dynamic) }
  .data : { *(.data) }
  .rela.dyn : { *(.rela.*) }
  .interp : { *(.interp) }
  .hash : { *(.hash) }
  .gnu.hash : { *(.gnu.hash) }
  .dynsym : { *(.dynsym) }
  .dynstr : { *(.dynstr) }
  .debug_foo : { *(.debug_foo) }
  .shstrtab : { *(.shstrtab) }
  .symtab : { *(.symtab) }
  .strtab : { *(.strtab) }
  /DISCARD/ : { *(*) }
}
