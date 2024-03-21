L0:	.byte	L1-L0
	.byte	L2-L0
	.byte	L3-L0
	.align	1
L1:	mov.l	1f,r1
	.uses	L1
	jmp	@r1
	nop
L2:	mov.l	1f,r1
	.uses	L2
	jmp	@r1
	nop
L3:	nop
	.align	2
1:	.long	2f
2:
