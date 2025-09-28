SECTIONS
{
  . = 0x1234000000000;
  .dynamic : { *(.dynamic) }
  . = 0x1234000001000;
  .hash : { *(.hash) }
  .dynsym : { *(.dynsym) }
  .dynstr : { *(.dynstr) }
  . = 0x1234000002000;
  .rel.dyn : { *(.rel.dyn) }

  . = 0x1234000003000;
  .init : { *(.init) }
  . = 0x1234000004000;
  .fini : { *(.fini) }
  .text : { *(.text) }
  .MIPS.stubs : { *(.MIPS.stubs) }

  . = 0x1235000000000;
  HIDDEN (_gp = ALIGN (16) + 0x7ff0);
  .got : { *(.got) }
  .data : { *(.data) }

  /DISCARD/ : { *(.MIPS.abiflags) *(.gnu.attributes) }
}
