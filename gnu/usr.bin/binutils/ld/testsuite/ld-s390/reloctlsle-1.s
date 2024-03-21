	.text
	.globl	_start
_start:
	larl	%r0,bar

	.section .tbss,"awT",@nobits
	.align	8
foo:	.zero	8
	.zero	8

	.data
	.align	8
	.quad	foo@NTPOFF
bar:	.quad	foo@NTPOFF+8
