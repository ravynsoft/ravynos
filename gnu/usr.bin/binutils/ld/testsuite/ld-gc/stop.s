	.text
	.globl _start
_start:
	.dc.a	0

	.section	_foo,"aw",%progbits
foo:
	.long	1
