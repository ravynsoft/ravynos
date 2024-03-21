SECTIONS
{
  .text : { *(.text) }
  .hash : { *(.hash) }
  .dynsym : { *(.dynsym) }
  .dynstr : { *(.dynstr) }
  .rela.dyn : { *(.rela.*) }
  . = ALIGN (0x1000);
  .got : { *(.got) }
  . = ALIGN (0x1000);
  .data : { *(.data) }
  .dynamic : { *(.dynamic) }
  .symtab : { *(.symtab) }
  .strtab : { *(.strtab) }
  .shstrtab : { *(.shstrtab) }
  /DISCARD/ : { *(*) }
}
