ENTRY(_start)
MEMORY
{
  RAM	: ORIGIN = 0x0000, LENGTH = 0x0FFF
  ROM	: ORIGIN = 0x1000, LENGTH = 0x0FFF
}

SECTIONS
{
  .data : { *(.data) } > RAM AT>ROM

  .text : { *(.text) } > ROM

  .bss : { *(.bss) } > RAM
}
