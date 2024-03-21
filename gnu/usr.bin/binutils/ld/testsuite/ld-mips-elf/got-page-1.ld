SECTIONS
{
  . = 0x80000;
  .interp : { *(.interp) }
  .hash : { *(.hash) }
  .dynsym : { *(.dynsym) }
  .dynstr : { *(.dynstr) }

  . = ALIGN (0x400);
  .rel.dyn : { *(.rel.dyn) }

  . = ALIGN (0x400);
  .MIPS.stubs : { *(.MIPS.stubs) }

  . = ALIGN (0x400);
  .text : { *(.text) }

  . = ALIGN (0x10000);
  HIDDEN (_gp = . + 0x7ff0);
  .got : { *(.got) }

  . = ALIGN (0x400);
  .bss : { *(.bss .bss.*) }

  /DISCARD/ : { *(.reginfo) *(.MIPS.abiflags) *(.MIPS.options) }
}

VERSION
{
  { local: *; };
}
