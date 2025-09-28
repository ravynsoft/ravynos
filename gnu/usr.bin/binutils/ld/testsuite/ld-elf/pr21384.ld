SECTIONS
{
  .data :
  {
    fdata = .;
    *(.data)
    edata = .;
  }
  .dynamic : { *(.dynamic) }
  .hash : { *(.hash) }
  .gnu.hash : { *(.gnu.hash) }
  .dynsym : { *(.dynsym) }
  .dynstr : { *(.dynstr) }
  .gnu.version : { *(.gnu.version) }
  .gnu.version_d : { *(.gnu.version_d) }
  .shstrtab : { *(.shstrtab) }
  .symtab : { *(.symtab) }
  .strtab : { *(.strtab) }
  .plt : { *(.plt) }
  .got.plt : { *(.got.plt) }
  .got : { *(.got) }
  /DISCARD/ : { *(*) }
}
