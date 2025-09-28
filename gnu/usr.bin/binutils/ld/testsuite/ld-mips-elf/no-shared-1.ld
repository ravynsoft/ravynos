SECTIONS
{
  . = 0x50000;
  .text : { *(.text) }
  .MIPS.stubs : { *(.MIPS.stubs) }

  . = 0x60000;
  .data : { *(.data) }
  HIDDEN (_gp = ALIGN (16) + 0x7ff0);
  .got : { *(.got) }
}
