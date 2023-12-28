 .section .toc,"aw",@progbits
.L0:
 .quad x

 .data
x:
 .quad f1

 .globl f1
 .type f1,@function
 .text
f1:
 addis 2,12,.TOC.-f1@ha
 addi 2,2,.TOC.-f1@l
 .localentry f1,.-f1
 mflr 0
 stdu 1,-32(1)
 std 0,48(1)
 bl f1
 ld 3,.L0@toc(2)
 bl f2
 nop
 ld 3,x@got(2)
 bl f3
 nop
 bl f4
 nop
 bl f5
 nop
 ld 0,48(1)
 addi 1,1,32
 mtlr 0
 blr
 .size f1,.-f1

 .globl f5
 .type f5,@function
f5:
 .localentry f5,1
 blr
 .size f5,.-f5
