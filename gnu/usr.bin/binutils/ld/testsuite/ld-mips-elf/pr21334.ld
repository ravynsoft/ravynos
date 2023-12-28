SECTIONS
{
  .text : { *(.text) }
  HIDDEN (_gp = ALIGN (16) + 0x7ff0);
  .got : { *(.got) }
  .symtab : { *(.symtab) }
  .strtab : { *(.strtab) }
  /DISCARD/ : { *(*) }
}
