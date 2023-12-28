 .ifdef HPUX
v_i .comm 4
 .else
  .comm v_i,4,4
 .endif

 .data
 .dc.a v_i

 .globl main
 .globl _main
 .globl start
 .globl _start
 .globl __start
 .text
main:
_main:
start:
_start:
__start:
 .dc.a 0
