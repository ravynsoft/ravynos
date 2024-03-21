OUTPUT_FORMAT(elf32-i386)
ENTRY(start)
SECTIONS
{
  .text : { *(.text*) }
  .data : { *(.data.*) }
  .bss : { *(.bss.*) }
}
