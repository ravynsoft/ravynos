SECTIONS
{
  . = 0;
  .reginfo : { *(.reginfo) }

  . = ALIGN (0x400);
  .dynamic : { *(.dynamic) }
  .dynsym : { *(.dynsym) }
  .dynstr : { *(.dynstr) }

  . = ALIGN (0x400);
  .rel.dyn : { *(.rel.dyn) }
  .foo : { *(.foo) }
  .text : { *(.text) }

  . = ALIGN (0x400);
  .MIPS.stubs : { *(.MIPS.stubs) }

  . = ALIGN (0x10000);
  .data : { *(.data) }

  . = ALIGN (0x400);
  HIDDEN (_gp = . + 0x7ff0);
  .got : { *(.got) }
}

VERSION {
  V2 { global: foo; local: *; };
}
