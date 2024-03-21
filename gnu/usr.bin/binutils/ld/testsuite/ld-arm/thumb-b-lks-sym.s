@ Test to ensure that the b to linker script symbol isn't changed to other format.


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
        b      extFunc
