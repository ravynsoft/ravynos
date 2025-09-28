 .section .entry, "xa"
 .align 5
 .globl _reset
 .type _reset, @function
_reset:
 tail _start
 .size _reset, . - _reset
