MEMORY
{
  text (rx) : ORIGIN = 0x10000000, LENGTH = 0x10000
  data (w) : ORIGIN = 0x7fff7fe8, LENGTH = 0x10000
}
SECTIONS
{
  .dynamic : { *(.dynamic) } >text
  .hash : { *(.hash) } >text
  .dynsym : { *(.dynsym) } >text
  .dynstr : { *(.dynstr) } >text
  .rel.plt : { *(.rel.plt) } >text
  .plt : { *(.plt) } >text
  .text : { *(.text) } >text
  .interp : { *(.interp) } >text
  .got.plt : { *(.got.plt) } >data
  .got : { *(.got) } >data
  .symtab : { *(.symtab) }
  .strtab : { *(.strtab) }
  .shstrtab : { *(.shstrtab) }
  /DISCARD/ : { *(*) }
}
