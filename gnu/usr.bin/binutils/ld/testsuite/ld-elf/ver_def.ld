SECTIONS
{
  .hash : { *(.hash) }
  .dynsym : { *(.dynsym) }
  .dynstr : { *(.dynstr) }
  .gnu.version : { *(.gnu.version) }
  .gnu.version_d : { *(.gnu.version_d) }
  .dynamic : { *(.dynamic) }
  .data : { *(.data) }
  .symtab : { *(.symtab) }
  .strtab : { *(.strtab) }
  .shstrtab : { *(.shstrtab) }
  .plt : { *(.plt) }
  .got.plt : { *(.got.plt) }
  .got : { *(.got) }
  /DISCARD/ : { *(*) }
}
