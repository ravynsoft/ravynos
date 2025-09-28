ENTRY (foo);
SECTIONS
{
  .text : { *(.text) }
  .got : { *(.got.plt) *(.got) }
  .dynamic : { *(.dynamic) }
  .rela.got : { *(.rela.got) }
  .plt : { *(.plt) }
  .hash : { *(.hash) }
  .dynsym : { *(.dynsym) }
  .dynstr : { *(.dynstr) }
  .shstrtab : { *(.shstrtab) }
  .symtab : { *(.symtab) }
  .strtab : { *(.strtab) }
  /DISCARD/ : { *(*) }
}
