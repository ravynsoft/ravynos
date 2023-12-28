      .text
      .global _start
_start:
      .type   _func, @function
_func:
      mov     L001,A1
      nop
A:
      mov     L001,A1
BOTTOM:
      .balign 0x8
      add     D0,D1
      .size   _func, .-_func

      .data
L001:
