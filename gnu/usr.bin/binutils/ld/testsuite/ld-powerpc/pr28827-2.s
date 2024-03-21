 .globl _start
 .text
_start:
 b far1
 b far2

 .section .far1,"ax",@progbits
far1:
 beq far2
 b _start

 .section .far2,"ax",@progbits
far2:
 bne far1
 b _start
