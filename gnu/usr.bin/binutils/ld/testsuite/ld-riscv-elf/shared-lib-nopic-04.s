        .option nopic
	.text
	.align  1

	call	foo_default
	jal	foo_default


	.globl  foo_default
	.type   foo_default, @function
foo_default:
	nop
	ret
	.size   foo_default, .-foo_default
