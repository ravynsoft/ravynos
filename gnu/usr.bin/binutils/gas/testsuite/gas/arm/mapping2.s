        .syntax unified
        .arch armv7-a
        .fpu softvfp
	.version "dfg"
        .thumb
        .text
        .align  2
        .global main
        .thumb
        .thumb_func
        .type   main, %function
main:
        push    {r4, lr}
foo:
        pop     {r4, lr}
        bx      lr
        .size   main, .-main
        .ident  ""

	nop
