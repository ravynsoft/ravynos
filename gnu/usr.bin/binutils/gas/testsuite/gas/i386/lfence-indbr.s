	.text
_start:
	call	*%edx
	jmp	*%edx
	call	*(%edx)
	jmp	*(%edx)
	call	*foo
	jmp	*foo
