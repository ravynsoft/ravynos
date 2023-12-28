OUTPUT_ARCH(mmix)
OUTPUT_FORMAT("mmo")
ENTRY(Main)
SECTIONS
{
  .text 0x100 + SIZEOF_HEADERS :
  { *(.text); Main = _start; }

  .MMIX.reg_contents :
  { *(.MMIX.reg_contents); }
}
