PHDRS
{
  header PT_PHDR PHDRS ;
  image  PT_LOAD PHDRS;
}

SECTIONS
{
  . = 0x1000;
  .text : { *(.text) *(.rodata.brlt) } :image :header
  /DISCARD/ : { *(.dropme) *(.reginfo) *(.MIPS.abiflags) *(.MIPS.options) }
}
