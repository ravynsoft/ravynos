SECTIONS
{
  . = 0x80000;
  .interp : { *(.interp) }
  .hash : { *(.hash) }
  .dynsym : { *(.dynsym) }
  .dynstr : { *(.dynstr) }

  . = ALIGN (0x400);
  .rel.dyn : { *(.rel.dyn) }

  . = ALIGN (0x400) + 0x400;
  .text : { *(.text) }

  . = ALIGN (0x10000);
  HIDDEN (_gp = . + 0x7ff0);
  .got : { *(.got) }

  . = ALIGN (0x400);
  .tdata : { *(.tdata) }

  /DISCARD/ : { *(.reginfo) *(.MIPS.abiflags) }
}

VERSION
{
  { local: *; };
}
