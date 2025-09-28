# old style ELFv1, with dot-symbols
 .globl my_func, .my_func
 .type .my_func, @function
 .section .opd, "aw", @progbits
my_func:
 .quad .my_func, .TOC.@tocbase, 0
 .size my_func, . - my_func

 .text
.my_func:
 blr
 .size .my_func, . - .my_func
