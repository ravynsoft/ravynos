SECTIONS
{
  . = 0x10000000;
  .interp : { *(.interp) }
  .reginfo : { *(.reginfo) }
  .hash : { *(.hash) }
  . = ALIGN(0x1000);
  .dynsym : { *(.dynsym) }
  . = ALIGN(0x1000);
  .dynstr : { *(.dynstr) }
  . = ALIGN(0x1000);
  .dynamic : { *(.dynamic) }
  . = ALIGN(0x1000);
  .rel.dyn : { *(.rel.dyn) }
  . = ALIGN(0x1000);
  .rel.plt : { *(.rel.plt) }
  . = 0x10100000;
  .plt : { *(.plt) }
  . = ALIGN(0x1000);
  .MIPS.stubs : { *(.MIPS.stubs) }
  . = ALIGN(0x1000);
  .text.a : { *(.text.a) }
  . = ALIGN(0x1000);
  .text.b : { *(.text.b) }
  . = ALIGN(0x1000);
  .text.c : { *(.text.c) }
  . = ALIGN(0x1000);
  .text.d : { *(.text.d) }
  . = 0x10200000;
  .got.plt : { *(.got.plt) }
  . = ALIGN(0x1000);
  .data : { *(.data) }
  . = ALIGN(0x1000);
  _gp = . + 0x7ff0;
  .got : { *(.got) }
  . = ALIGN(0x1000);
  .rld_map : { *(.rld_map) }

  /DISCARD/ : { *(.MIPS.abiflags) *(.gnu.attributes) }
}
