ENTRY(v1)
SECTIONS
{
  . = 0xffe00000;
  .global : { *(.global) }
  . = 0x4018280;
  .func2 : { *(.func2) }
  . = 0x4005000;
  .func1 : { *(.func1) }
  . = 0x4015000;
  .func3 : { *(.func3) }
  .data : { *(.data) }
  .rodata : { *(.rodata) }
}