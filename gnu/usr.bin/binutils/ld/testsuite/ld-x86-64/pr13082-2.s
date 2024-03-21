	.text
	.globl _start
_start:
	lea	.Ljmp(%rip), %rax
	jmp	*(%rax)
	.section	.data.rel.ro.local,"aw",@progbits
	.align	8
.Ljmp:
	.quad _start
