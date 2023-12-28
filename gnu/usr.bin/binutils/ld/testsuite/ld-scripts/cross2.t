NOCROSSREFS ( .text .data )
SECTIONS
{
  .text : { *(.text) *(.text.*) *(.pr) *(.opd) }
  .data : { *(.data) *(.data.*) *(.sdata) *(.rw) *(.tc0) *(.tc) *(.toc) }
}
