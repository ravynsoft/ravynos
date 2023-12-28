SECTIONS
{
  .hash : { *(.hash) }
  .gnu.hash : { *(.gnu.hash) }
  .dynsym : { *(.dynsym) }
  .dynstr : { *(.dynstr) }
  .rel.dyn : { *(.rel.dyn) }
  .text : { *(.text) }
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
