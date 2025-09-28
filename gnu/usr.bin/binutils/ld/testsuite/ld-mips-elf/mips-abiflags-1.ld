SECTIONS
{
  .MIPS.abiflags : { *(.foo) }
  /DISCARD/ : { *(*) }
}
