NOCROSSREFS_TO(.data .text)

SECTIONS
{
  .text : { *(.text) *(.text.*) *(.opd) }
  .data : { *(.data) *(.data.*) *(.sdata) *(.toc) }
  .bss : { *(.bss) *(COMMON) }
  /DISCARD/ : { *(*) }
}
