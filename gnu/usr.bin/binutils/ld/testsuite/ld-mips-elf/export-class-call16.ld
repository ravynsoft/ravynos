ENTRY (foo);
SECTIONS
{
  .text : { *(.text) }
  HIDDEN (_gp = ALIGN(16) + 0x7ff0);
  .got : { *(.got) }
  .dynamic : { *(.dynamic) }
  .hash : { *(.hash) }
  .dynsym : { *(.dynsym) }
  .dynstr : { *(.dynstr) }
  .pdr : { *(.pdr) }
  .shstrtab : { *(.shstrtab) }
  .symtab : { *(.symtab) }
  .strtab : { *(.strtab) }
  /DISCARD/ : { *(*) }
}
