	.cfi_startproc
	mov	w1, 26215
	movk	w1, 0x6666, lsl 16
	smull	x1, w0, w1
	asr	x1, x1, 34
	sub	w1, w1, w0, asr 31
	add	w1, w1, w1, lsl 2
	sub	w0, w0, w1, lsl 1
	ret
	.cfi_endproc
