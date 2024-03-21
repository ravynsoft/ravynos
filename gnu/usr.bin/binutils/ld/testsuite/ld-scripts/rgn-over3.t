/* Memory region overflow tests: two regions, each too small for the single
   section placed there. */

MEMORY {
  bss (rwx) : ORIGIN = 0, LENGTH = 0
  r1  (rwx) : ORIGIN = 0x1000, LENGTH = 8
  r2  (rwx) : ORIGIN = 0x2000, LENGTH = 8
}
_start = 0x1000;
SECTIONS {
  .bss  : { *(.bss)  } > bss
  .text : { *(.txt) } > r1
  .data : { *(.dat) } > r2
  /DISCARD/ : { *(*) }
}
