SECTIONS
{
  . = 0x1100;
  .rodata : { *(.rodata .rodata.* .gnu.linkonce.r.*) }
  . = 0x1200;
  .data : { *(.data .data.* .gnu.linkonce.d.*) }
  .junk : { *(*) }
}
