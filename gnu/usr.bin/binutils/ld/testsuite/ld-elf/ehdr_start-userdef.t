SECTIONS
{
  . = 0x10000000;
  .text : { *(.text) }

  __ehdr_start = 0x12345678;

  . = 0x20000000;
  .rodata : { *(.rodata) }
}
