 .text
 lis 3,x@ha
 addi 3,3,x@l
 .org 0x10000
 lis 3,x@ha
 addi 3,3,x@l
 .org 0x20000
 lis 3,x@ha
 addi 3,3,x@l

 .org 0x2fff4
 bcl 20,31,.+4
0:
 mflr 9
 addis 9,9,x-0b@ha
 addi 9,9,x-0b@l

 .section .bss,"aw",@nobits
x: .space 4
