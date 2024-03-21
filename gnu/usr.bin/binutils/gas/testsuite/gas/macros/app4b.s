#NO_APP
 .globl label_d
label_d:
 .long 21
#APP
 .if 0
#NO_APP
 .err
 .globl label_x
label_x:
#APP
 .endif
