 .section .toc,"aw"
x4t:
 .quad x4
x5t:
 .quad x5
x6t:
 .quad x6

 .section .sdata,"aw"
x1:
 .byte 1
x2:
 .byte 2
x3:
 .byte 3
x4:
 .byte 4
x5:
 .byte 5
x6:
 .byte 6

 .globl _start
 .text
_start:
# no need for got entry, optimise to nop,addi
 addis 9,2,x1@got@ha
 ld 9,x1@got@l(9)
# must keep got entry, optimise to nop,addi,ld
 addis 4,2,x2@got@ha
 addi 5,4,x2@got@l
 ld 6,0(5)

# no need for toc entry, optimise to nop,addi
 addis 9,2,x4t@toc@ha
 ld 9,x4t@toc@l(9)
# must keep toc entry, optimise to nop,addi,ld
# if we had a reloc tying the ld to x5/x5t then we could throw away
# the toc entry and optimise to nop,nop,addi
 addis 4,2,x5t@toc@ha
 addi 5,4,x5t@toc@l
 ld 6,0(5)
