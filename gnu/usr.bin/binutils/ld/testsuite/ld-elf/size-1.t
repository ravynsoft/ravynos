SECTIONS
{
  .text : { *(.text) }
  .data : { *(.data) }
  .bss : { *(.bss) }
  .tdata : { *(.tdata) }
  .tbss : { *(.tbss) }
  .map : {
    LONG (SIZEOF (.text))
    LONG (SIZEOF (.data))
    LONG (SIZEOF (.bss))
    LONG (SIZEOF (.tdata))
    LONG (SIZEOF (.tbss))
  }
  /DISCARD/ : { *(*) }
}
