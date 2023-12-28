        .syntax unified
        .cpu cortex-m4
        .fpu softvfp
        .thumb
        .file   "m.c"
        .text
        .align  2
        .global main
        .thumb
        .thumb_func
        .type   main, %function
main:
        push    {r7, lr}
        add     r7, sp, #0
        bl      foo
        mov     r3, r0
        mov     r0, r3
        pop     {r7, pc}
