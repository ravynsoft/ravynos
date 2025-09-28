	.cfi_startproc
	cmp	w0, 1000
	bgt	.L4
	ret
.L4:
	b	foo
	.cfi_endproc
