NOCROSSREFS_TO(.data .nocrossrefs)

SECTIONS
{
  .text : { *(.text) *(.text.*) *(.opd) }
  .nocrossrefs : { *(.nocrossrefs) }
  .data : { *(.data) *(.data.*) *(.sdata) *(.toc) }
  .bss : { *(.bss) *(COMMON) }
  /DISCARD/ : { *(*) }
}
