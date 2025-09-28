SECTIONS
{
  . = 0x1234000000000;
  .dynamic : { *(.dynamic) }
  .hash : { *(.hash) }
  .dynsym : { *(.dynsym) }
  .dynstr : { *(.dynstr) }
  .rel.dyn : { *(.rel.dyn) }

  . = 0x1235000000000;
  .text : { *(.text) }
  .MIPS.stubs : { *(.MIPS.stubs) }

  . = 0x1236000000000;
  .data : { *(.data) }
  HIDDEN (_gp = ALIGN (16) + 0x7ff0);
  .got : { *(.got) }

  /DISCARD/ : { *(.MIPS.abiflags) *(.gnu.attributes) }
}
