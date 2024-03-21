	.text
	.globl _start
_start:
	mov	.Ljmp(%eax), %eax
	jmp	*(%eax)
	.section	.data.rel.ro.local,"aw",@progbits
	.weak func
	.align	8
.Ljmp:
	.long func
