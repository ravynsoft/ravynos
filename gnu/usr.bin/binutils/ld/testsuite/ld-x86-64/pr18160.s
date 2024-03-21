	.section	.text.start,"ax",@progbits
	.globl	start
	.type	start, @function
start:
	.cfi_startproc
	jmp	foo
	.cfi_endproc
	.size	start, .-start
	.section	.text.foo,"ax",@progbits
	.globl	foo
	.type	foo, @function
foo:
	.cfi_startproc
	ret
	.cfi_endproc
	.size	foo, .-foo
