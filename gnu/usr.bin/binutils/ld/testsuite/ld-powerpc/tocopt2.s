 .globl xt
 .section .toc,"aw"
xt:
 .quad x

 .globl _start
 .text
_start:
 addis 9,2,xt@toc@ha
 ld 9,xt@toc@l(9)
