	.globl main
	.globl start
	.globl _start
	.globl __start
	.text
main:
start:
_start:
__start:
	.long	0
	.data
	.long	0
	.section .data.rel.ro,"aw",%progbits
	.long	0
