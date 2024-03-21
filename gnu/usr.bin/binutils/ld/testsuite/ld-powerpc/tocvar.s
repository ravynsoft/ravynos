 .section .opd,"aw",@progbits
 .global _start
 .type _start,@function
_start:
 .quad .L_start, .TOC.@tocbase, 0

 .text
.L_start:
 lwz 3,x@toc(2)
 b _start
 .size _start,.-.L_start

 .section .toc,"aw",@progbits
 .global x
 .type x,@object
x: .long 0
 .size x,.-x
