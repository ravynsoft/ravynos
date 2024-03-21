 .text
 .global odd, odd2, aligned
 .dc.b 1
odd:
 .rept 65536
 .dc.l 0
 .endr
odd2:
 .dc.b 2,3,4
aligned:
 .rept 65536
 nop
 .endr
