 .text
 .globl ext
ext:
0:
 addis 2,12,.TOC.-0b@ha
 addi 2,2,.TOC.-0b@l
 .localentry ext,.-0b
 nop
 blr
