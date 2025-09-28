	.thumb
	.syntax unified
	.cpu cortex-m7
	.fpu fpv5-d16
	.eabi_attribute 27, 3
        .text
        .align  2
        .global main
        .thumb
        .thumb_func
        .type   main, %function



main:
	bl foo
