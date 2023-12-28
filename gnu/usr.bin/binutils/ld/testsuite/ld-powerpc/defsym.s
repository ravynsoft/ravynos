 .text
 .globl _start
_start:
 bl foo
 bl bar
 nop

 .globl foo
 .type foo,@function
foo:
 addis 2,12,.TOC.-foo@ha
 addi 2,2,.TOC.-foo@l
 .localentry foo,.-foo
 blr
 .size foo,.-foo

 .data
 .dc.a foo
 .dc.a bar
