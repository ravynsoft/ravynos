	.text
	.intel_syntax noprefix
	.arch .no87
stN:
	mov	eax, st
	mov	eax, st(7)
	mov	eax, st ( 7 )
	mov	eax, x(7)
	.p2align 4,0
