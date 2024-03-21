SECTIONS
{
  . = 0x80000;
  .interp : { *(.interp) }
  .hash : { *(.hash) }
  .dynsym : { *(.dynsym) }
  .dynstr : { *(.dynstr) }
  .rel.dyn : { *(.rel.dyn) }
  .text : { *(.text) }

  . = ALIGN (0x10000);
  HIDDEN (_gp = . + 0x7ff0);
  .got : { *(.got) }

  .symtab : { *(.symtab) }
  .strtab : { *(.strtab) }
  .shstrtab : { *(.shstrtab) }

  /DISCARD/ : {
    *(.reginfo) *(.MIPS.abiflags) *(.MIPS.stubs) *(.gnu.attributes)
  }
}
