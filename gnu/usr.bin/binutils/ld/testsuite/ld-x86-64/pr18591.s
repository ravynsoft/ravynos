	.hidden foo
	.comm pad,0x80000000,8
	.comm foo,8,8
	.text
	.globl	bar
	.type	bar, @function
bar:
	movq	foo@GOTPCREL(%rip), %rax
