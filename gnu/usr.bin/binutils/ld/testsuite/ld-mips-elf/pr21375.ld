SECTIONS
{
  .dynamic : { *(.dynamic) }
  .hash : { *(.hash) }
  .dynsym : { *(.dynsym) }
  .dynstr : { *(.dynstr) }
  .gnu.version : { *(.gnu.version) }
  .gnu.version_d : { *(.gnu.version_d) }
  .gnu.version_r : { *(.gnu.version_r) }
  /* Align up to account for traditional vs IRIX target differences
     with the alignment of dynamic sections.  This way GOT addresses
     work out the same.  */
  .text : ALIGN (512) { *(.text*) }
  HIDDEN (_gp = ALIGN (16) + 0x7ff0);
  .got : { *(.got) }
  .symtab : { *(.symtab) }
  .strtab : { *(.strtab) }
  .shstrtab : { *(.shstrtab) }
  /DISCARD/ : { *(*) }
}
