	.globl main
	.globl start
	.globl _start
	.globl __start
	.text
main:
start:
_start:
__start:
	.byte 0

	.section .tbss,"awT",%nobits
	.p2align 10
	.type	tbss, %object
	.size	tbss, 1024
tbss:
	.zero	1024
