#NO_APP
 .text
 .macro foo
 .globl label_a
label_a:
 .long 42
 .endm
#APP
 foo
 .globl label_b
label_b:
 .long 56
#NO_APP
 .globl label_c
label_c:
 .long 78
