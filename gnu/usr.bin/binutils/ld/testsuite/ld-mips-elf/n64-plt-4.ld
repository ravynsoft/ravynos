MEMORY
{
  text (rx) : ORIGIN = 0xffffffff10000000, LENGTH = 0x10000
  data (w) : ORIGIN = 0xffffffff7fff8000, LENGTH = 0x10000
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
  .rld.map : { *(.rld.map) } >data
  .got : { *(.got) } >data
  .symtab : { *(.symtab) }
  .strtab : { *(.strtab) }
  .shstrtab : { *(.shstrtab) }
  /DISCARD/ : { *(*) }
}
