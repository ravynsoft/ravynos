SECTIONS
{
  . = 0x40000;
  .interp : { *(.interp) }
  .dynsym : { *(.dynsym) }
  .dynstr : { *(.dynstr) }
  .dynamic : { *(.dynamic) }
  .reginfo : { *(.reginfo) }
  .hash : { *(.hash) }
  . = ALIGN (0x400);
  .text : { *(.text) *(.mips16.*) }
  .MIPS.stubs : { *(.MIPS.stubs) }
  . = ALIGN (0x400);
  .rel.plt : { *(.rel.plt) }
  .rel.dyn : { *(.rel.dyn) }
  . = 0x50000;
  HIDDEN (_gp = . + 0x7ff0);
  .got : { *(.got) }
  . = 0x50400;
  .data : { *(.data) }
  .rld_map : { *(.rld_map) }
}
