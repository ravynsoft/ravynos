SECTIONS
{
  . = 0x40000 + SIZEOF_HEADERS;
  .interp : { *(.interp) }

  . = 0x41000;
  .reginfo : { *(.reginfo) }
  .hash : { *(.hash) }
  .dynsym : { *(.dynsym) }
  .dynstr : { *(.dynstr) }

  . = 0x42000;
  .dynamic : { *(.dynamic) }

  . = 0x43000;
  .rel.dyn : { *(.rel.dyn) }
  .rel.plt : { *(.rel.plt) }
  .plt : { *(.plt) }

  . = 0x44000;
  .text : { *(.text) }
  .MIPS.stubs : { *(.MIPS.stubs) }
  .MIPS.options : { *(.MIPS.options) }

  . = 0x80000;
  .rld_map : { *(.rld_map) }

  . = 0x81000;
  .got.plt : { *(.got.plt) }

  . = 0xa0000;
  HIDDEN (_gp = . + 0x7ff0);
  .got : { *(.got) }

  . = 0xa1000;
  .data : { *(.data) }

  . = 0xa2000;
  .bss : { *(.dynbss) }

  /DISCARD/ : { *(.MIPS.abiflags) *(.gnu.attributes) }
}
