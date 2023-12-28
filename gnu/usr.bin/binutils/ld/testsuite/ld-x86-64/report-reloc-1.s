	.text
	.globl _start
	.type ifunc, @gnu_indirect_function
_start:
	lea	.Ljmp(%rip), %rax
ifunc:
	jmp	*(%rax)
	.section	.data.rel.ro.local,"aw",@progbits
	.align	8
.Ljmp:
	.quad ifunc
	.quad _start
