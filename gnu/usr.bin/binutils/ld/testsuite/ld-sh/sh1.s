	.text
foo:
L1:	
	mov.l	L2,r0
	.uses	L1
	jsr	@r0
	nop
	.uses	L1
	jmp	@r0
	nop
	rts
	nop
	.align	2
L2:
	.long	bar
bar:
	rts
	.align	4
