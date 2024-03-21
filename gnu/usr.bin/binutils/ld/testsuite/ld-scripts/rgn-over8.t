/* Memory region overflow tests: bss to LMA doesn't cause overflow.  */

MEMORY {
  rom (rwx) : ORIGIN = 0, LENGTH = 2048
  ram (rwx) : ORIGIN = 0x1000, LENGTH = 2048
}
_start = 0x0;
SECTIONS {
  .text : { *(.text) } >rom AT>rom
  .data : { *(.data) } >ram AT>rom
  .bss : { *(.bss) } >ram AT>rom
  /DISCARD/ : { *(*) }
}
