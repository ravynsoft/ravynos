NOCROSSREFS_TO(.nocrossrefs .data)

SECTIONS
{
  .text : { *(.text) *(.text.*) }
  .nocrossrefs : { *(.nocrossrefs) }
  .data : { *(.data) *(.data.*) *(.sdata) *(.opd) *(.toc) }
  .bss : { *(.bss) *(COMMON) }
  /DISCARD/ : { *(*) }
}
