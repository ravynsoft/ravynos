SECTIONS
{
  .linkorder : { *(.linkorder.*) }
  .text : { *(.text) }
  /DISCARD/ :
  {
    *(.reginfo) *(.MIPS.abiflags) *(.MIPS.options) *(.trampolines)
  }
}
