	.section .text.live,"ax",%progbits
	.globl live
live:
	.byte 0

	.section .stack_sizes,"o",%progbits,.text.live,unique,0
	.byte 1

	.section .text.dead,"ax",%progbits
	.globl dead
dead:
	.byte 1

	.section .stack_sizes,"o",%progbits,.text.dead,unique,1
	.byte 2

	.section .text.main,"ax",%progbits
	.globl _start
_start:
	.byte 0
	.section .note,"",%note
	.dc.a live
