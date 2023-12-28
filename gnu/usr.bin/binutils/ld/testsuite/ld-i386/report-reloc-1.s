	.text
	.globl _start
	.type ifunc, @gnu_indirect_function
_start:
	lea	.Ljmp@GOTOFF(%ebx), %eax
ifunc:
	jmp	*(%eax)
	.section	.data.rel.ro.local,"aw",@progbits
	.align	4
.Ljmp:
	.long ifunc
	.long _start
