NOCROSSREFS_TO(.text .data)

SECTIONS
{
  .text : { *(.text) *(.text.*) *(.opd) }
  .data : { *(.data) *(.data.*) *(.sdata) *(.toc) }
  .bss : { *(.bss) *(COMMON) }
  /DISCARD/ : { *(*) }
}
