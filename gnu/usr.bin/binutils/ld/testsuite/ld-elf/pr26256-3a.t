SECTIONS
{
  .rodata :
    {
      BYTE(0)
      *(.rodata.foo)
      *(.rodata.bar)
      BYTE(4)
      *(.rosection)
      BYTE(7)
    }
  .text : {*(.text.bar) *(.text.foo)}
  /DISCARD/ :
  {
    *(.reginfo) *(.MIPS.abiflags) *(.MIPS.options) *(.trampolines)
  }
}
