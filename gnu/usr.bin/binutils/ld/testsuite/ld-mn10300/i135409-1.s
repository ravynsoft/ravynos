       .text

       nop

        .global _start
_start:
        .type   _func, @function
_func:
        mov     L001,A1
        nop
A:
        mov     L001,A1
        .size   _func, . - _func


        .global _func2
_func2:
        .type   _func2, @function
        mov     L001,A1
        nop
        mov     L001,A1
        .size   _func2, . - _func2

        .global BOTTOM
BOTTOM:
	
        .data
L001:

