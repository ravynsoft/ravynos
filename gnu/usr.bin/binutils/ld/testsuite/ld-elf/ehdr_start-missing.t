SECTIONS
{
  . = 0x10000000;
  .text : { *(.text) }

  . = 0x20000000;
  .rodata : { *(.rodata) }
}
