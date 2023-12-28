	.text
	.globl _start
_start:
	lea	.Ljmp(%rip), %rax
	jmp	*(%rax)
	.section	.data.rel.ro.local,"aw",@progbits
	.weak func
	.align	8
.Ljmp:
	.quad func + 1
