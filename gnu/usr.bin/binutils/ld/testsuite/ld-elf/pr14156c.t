SECTIONS {
  .text : { *(.text) }
  .foo : { *(SORT_NONE(.foo)) }
  /DISCARD/ : { *(.*) }
}
