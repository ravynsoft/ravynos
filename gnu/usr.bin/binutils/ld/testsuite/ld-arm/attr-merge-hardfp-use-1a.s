        .syntax unified
        .cpu cortex-m4
        .fpu fpv4-sp-d16
        .thumb
        .text
        .align  2
        .global foo
        .thumb
        .thumb_func
        .type   foo, %function
foo:
	bx lr
