 .abiversion 1
 .global _start
 .type _start,@function
 .section ".opd","aw",@progbits
 .p2align 3
_start:
 .quad .L_start, .TOC.@tocbase, 0

 .text
.L_start:
 nop
.L1:
# tocsave in a function prologue
 .reloc .,R_PPC64_TOCSAVE,.L1
 nop

 nop
# tocsave on a call
 bl foo
 .reloc .,R_PPC64_TOCSAVE,.L1
 nop

 blr
 .size _start, .-.L_start
