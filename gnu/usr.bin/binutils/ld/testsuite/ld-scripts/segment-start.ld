SECTIONS
{
  PROVIDE (__executable_start = SEGMENT_START ("text-segment", 0x10000000));
  .text : { *(.text) }
  .data : { *(.data) }
  .bss : { *(.bss) }
  .loader : { *(.loader) }
  .symtab : { *(.symtab) }
  .strtab : { *(.strtab) }
  .shstrtab : { *(.shstrtab) }
  /DISCARD/ : { *(*) }
}
