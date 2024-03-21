        .syntax unified
        .cpu cortex-m4
        .fpu fpv4-sp-d16
        .thumb
        .text
        .global _start
        .thumb_func
        .type   _start, %function
_start:
	bl foo
