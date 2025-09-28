     .text
      .global _start
_start:
      add     A0, A1
_A:
      mov     L001, A0
_B:
      .balign 0x8
_C:
      nop
      .balign 0x10

       .type   _func, @function
_func:
       mov     L001, A1
       nop
_D:
       mov     L001, A1
BOTTOM:
       .size   _func, . - _func

      .data
L001:
