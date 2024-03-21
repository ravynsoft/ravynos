 .globl f3
 .type f3,@function
 .text
f3:
 addis 2,12,.TOC.-f3@ha
 addi 2,2,.TOC.-f3@l
 .localentry f3,.-f3
 blr
 .size f3,.-f3

 .globl f4
 .type f4,@function
 .text
f4:
 .localentry f4,0
 blr
 .size f4,.-f4
