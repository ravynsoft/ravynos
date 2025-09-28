@ Test to ensure that the bl to linker script symbol isn't changed to blx with immediate address.


        .syntax unified
        .cpu cortex-m3
        .fpu softvfp
        .thumb
        .file   "x.c"
        .text
        .align  2
        .global main
        .thumb
        .thumb_func
        .type   main, %function
main:
        push    {r7, lr}
        add     r7, sp, #0
        bl      extFunc
        pop     {r7, pc}
