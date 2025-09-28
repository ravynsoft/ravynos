	.section .stack_sizes,"o",%progbits,live,unique,0
live:
	.byte 1

	.section .stack_sizes,"o",%progbits,dead,unique,1
dead:
	.byte 2

	.section .text.main,"ax",%progbits
	.globl _start
_start:
	.byte 0
	.section .note,"",%note
	.dc.a live
