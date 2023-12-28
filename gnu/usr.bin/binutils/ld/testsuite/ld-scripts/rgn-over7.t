/* Memory region overflow tests: overflow r1 plus text/data collision.  */

MEMORY {
  bss (rwx) : ORIGIN = 0, LENGTH = 0
  r1  (rwx) : ORIGIN = 0x1000, LENGTH = 8
  r2  (rwx) : ORIGIN = 0x1008, LENGTH = 12
}
_start = 0x1000;
SECTIONS {
  .bss  : { *(.bss)  } > bss
  .text : { *(.txt) } > r1
  .data : { *(.dat) } > r2
  /DISCARD/ : { *(*) }
}
