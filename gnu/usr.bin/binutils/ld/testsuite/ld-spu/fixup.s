 .global _end
 .global _start
 .global glob
 .global after
 .global before
 .weak undef

 .section .text,"ax"
_start:
 stop


 .data
 .p2align 4
before:
 .long _end, 0, _start, after
 .long 0, 0, 0, glob
loc:
 .long 1,0,0,0
glob:
 .long 2,0,0,0
after:
 .long 0, 0, 0, before

