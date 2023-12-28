 .text
 .macro foo
 .globl label_a
label_a:
 .long 42
 .endm
 .include "app4b.s"
 foo
 .globl label_b
label_b:
 .long 56
