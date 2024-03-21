 .globl _start
 .section .opd,"aw",@progbits
_start:
 .quad .L_start, .TOC.@tocbase

 .text
.L_start:
 b _start
