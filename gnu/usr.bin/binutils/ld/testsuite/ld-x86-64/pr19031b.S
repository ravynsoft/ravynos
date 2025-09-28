	.text
	.globl g
	.type g, @function
g:
	movq f@GOTPCREL(%rip), %rax
	retq

	.globl h
	.type h, @function
h:
	leaq zed, %rax
	movslq zed, %rbx
	addq %rbx, %rax
	retq

	.data
zed:
        .long f - .
	.section	.note.GNU-stack
