# Test TLS IE to TLS LE relaxation for global symbols.
        .section        .tbss,"awT",@nobits
        .global x
        .align  2
        .type   x, %object
        .size   x, 4
x:
        .zero   4



        .text
        .global _start
        .type _start, @function
_start:
        mov     x0, 0
        mrs     x1, tpidr_el0
        adrp    x0, :gottprel:x
        ldr     x0, [x0, #:gottprel_lo12:x]
        ldr     w0, [x1, x0]
        ret

