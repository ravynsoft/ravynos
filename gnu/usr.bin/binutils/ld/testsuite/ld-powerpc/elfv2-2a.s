 .globl f1
 .type f1,@function
 .text
f1:
 addis 2,12,.TOC.-f1@ha
 addi 2,2,.TOC.-f1@l
 .localentry f1,.-f1
 blr
 .size f1,.-f1

 .globl f2
 .type f2,@function
 .text
f2:
 addi 2,12,.TOC.-f2
 .localentry f2,.-f2
 blr
 .size f2,.-f2

 .quad f1
 .quad f1@localentry
 .quad f2
 .quad f2@localentry
 .quad f3
 .quad f3@localentry
 .quad f4
 .quad f4@localentry
