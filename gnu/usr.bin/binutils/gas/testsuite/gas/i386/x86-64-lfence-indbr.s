	.text
_start:
	call	*%rdx
	jmp	*%rdx
	call	*(%rdx)
	jmp	*(%rdx)
	call	*foo
	jmp	*foo
