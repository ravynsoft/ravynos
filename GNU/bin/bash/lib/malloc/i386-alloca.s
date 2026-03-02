	.file "alloca.s"
	.text
	.align 4
	.def alloca; .val alloca; .scl 2; .type 044; .endef
	.globl alloca
alloca:
	popl %edx
	popl %eax
	addl $3,%eax
	andl $0xfffffffc,%eax
	subl %eax,%esp
	movl %esp,%eax
	pushl %eax
	pushl %edx
	ret
	.def alloca; .val .; .scl -1; .endef
