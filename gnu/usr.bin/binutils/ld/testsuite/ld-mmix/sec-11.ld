OUTPUT_ARCH(mmix)
ENTRY(Main)
SECTIONS
{
  .text 0x101 : /* Note the misaligned address; must trig a linker error.  */
  { *(.text*); Main = _start; }

  .MMIX.reg_contents :
  { *(.MMIX.reg_contents.linker_allocated); *(.MMIX.reg_contents); }
}
