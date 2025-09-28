/* Memory region overflow tests: overflow VMA but not LMA.  */

MEMORY {
  bss (rwx) : ORIGIN = 0, LENGTH = 0
  r1  (rwx) : ORIGIN = 0x1000, LENGTH = 8
  v1  (rwx) : ORIGIN = 0x2000, LENGTH = 24
}
_start = 0x1000;
SECTIONS {
  .bss  : { *(.bss)  } > bss
  .text : { *(.txt) } > r1 AT> v1
  .data : { *(.dat) } > r1 AT> v1
  /DISCARD/ : { *(*) }
}
