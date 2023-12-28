 .globl x
 .hidden x

 .section .toc,"aw"
 .p2align 3
.Lx:
 .quad x
.La:
 .quad a
.Lb:
 .quad b
.Lc:
 .quad c

 .data
 .p2align 3
x:
 .quad x
 .quad a
 .quad b
 .quad c

 .text
 .p2align 2
 .globl _start
 .type _start,@function
_start:
0:
 addis 2,12,.TOC.-0b@ha
 addi 2,2,.TOC.-0b@l
 .localentry _start,.-_start
 addis 3,2,.Lx@toc@ha
 ld 3,.Lx@toc@l(3)
 addis 4,2,.La@toc@ha
 ld 4,.La@toc@l(4)
 addis 5,2,.Lb@toc@ha
 ld 5,.Lb@toc@l(5)
 addis 6,2,.Lc@toc@ha
 ld 6,.Lc@toc@l(6)

 addis 7,2,x@got@ha
 ld 7,x@got@l(7)
 addis 8,2,a@got@ha
 ld 8,a@got@l(8)
 addis 9,2,b@got@ha
 ld 9,b@got@l(9)
 addis 10,2,c@got@ha
 ld 10,c@got@l(10)
 .size _start,.-_start
