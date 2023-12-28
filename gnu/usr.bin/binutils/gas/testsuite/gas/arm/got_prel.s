        .code   16
        .text
.Ltext0:
        .section        .text.foo,"ax",%progbits
        .align  2
        .global foo
        .code   16
        .thumb_func
        .type   foo, %function
foo:
        .fnstart
        ldr     r3, .L3
.LPIC0:
        add     r3, pc
        ldr     r3, [r3]
        ldr     r2, [r3]
        str     r0, [r3]
        mov     r0, r2
        bx      lr
        .align  2
.L3:
        .word   i(GOT_PREL) + (. - (.LPIC0+4))
        .fnend
