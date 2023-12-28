/* Memory region at test, >AT should propagate by default */

MEMORY {
  ram : ORIGIN = 0x10000, LENGTH = 0x100
  rom : ORIGIN = 0x20000, LENGTH = 0x200
}
_start = 0x1000;
SECTIONS {
  .text : { *(.text) } >ram AT>rom
  .data : { *(.data) } >ram /* default AT>rom */
  .bss : { *(.bss) } >ram
  /DISCARD/ : { *(*) }
}
