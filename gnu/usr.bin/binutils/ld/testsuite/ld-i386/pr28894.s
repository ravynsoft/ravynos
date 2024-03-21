	.text
	addl	foo@INDNTPOFF, %eax
	.section .tbss,"awT",@nobits
	.globl	foo
	.hidden foo
foo:
	.byte 0
