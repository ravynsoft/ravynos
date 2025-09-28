 .section .toc,"aw"
0:
 .quad x

 .globl _start
 .text
_start:
 addis 9,2,0b@toc@ha
 ld 9,0b@toc@l(9)
