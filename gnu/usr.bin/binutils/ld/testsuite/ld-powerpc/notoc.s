 .text
 .globl f1, f2, g1, g2, _start
 .weak ext
 .abiversion 2

f1:
 .localentry f1,1
 bl f1@notoc
 bl f2@notoc
 bl g1@notoc
 bl g2@notoc
 bl ext@notoc
 blr

g1:
 .localentry g1,1
 bl f2@notoc
 bl f1@notoc
 bl g2@notoc
 bl g1@notoc
 blr

f2:
0:
 addis 2,12,.TOC.-0b@ha
 addi 2,2,.TOC.-0b@l
 .localentry f2,.-0b
 bl f1
 nop
 bl f2
 nop
 bl g1
 nop
 bl g2
 nop
 bl ext
 nop
 blr

g2:
0:
 addis 2,12,.TOC.-0b@ha
 addi 2,2,.TOC.-0b@l
 .localentry g2,.-0b
 bl f2
 nop
 bl f1
 nop
 bl g2
 nop
 bl g1
 nop
 blr

_start:
 .cfi_startproc
 b _start
 .cfi_endproc
