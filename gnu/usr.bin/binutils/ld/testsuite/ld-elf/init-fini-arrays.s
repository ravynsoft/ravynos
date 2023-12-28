 .section .init_array.01000,"aw",%init_array
 .p2align 2
 .weak __init_array_start, ___init_array_start
 .dc.a __init_array_start
 .dc.a ___init_array_start

 .section .fini_array.01000,"aw",%fini_array
 .p2align 2
 .weak __fini_array_start, ___fini_array_start
 .dc.a __fini_array_start
 .dc.a ___fini_array_start

 .text
 .globl main, _main, start, _start, __start
main:
_main:
start:
_start:
__start:
 .dc.a 0
