/* Memory region overflow tests: one region, first output sect doesn't fit. */

MEMORY {
  bss (rwx) : ORIGIN = 0, LENGTH = 0
  r1  (rwx) : ORIGIN = 0x1000, LENGTH = 8
}
_start = 0x1000;
SECTIONS {
  .bss  : { *(.bss)  } > bss
  .text : { *(.txt) } > r1
  .data : { *(.dat) } > r1
  /DISCARD/ : { *(*) }
}
