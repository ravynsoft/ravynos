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
	.section	.data.rel.ro.local,"aw",%progbits
	.weak func
	.dc.a func
