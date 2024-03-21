SECTIONS
{
  . = SIZEOF_HEADERS;
  .text : { *(.text) }
  .data : { *(.data) }
  .bss : { *(.bss) *(COMMON) }
  .sbss : { *(.sbss) }
  .note : { *(.note) }
  /DISCARD/ : { *(.reginfo) *(.MIPS.abiflags) *(.MIPS.options) *(.trampolines) }
}
