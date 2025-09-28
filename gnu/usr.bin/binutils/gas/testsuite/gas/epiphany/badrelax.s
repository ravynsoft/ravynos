	.text
	.global xxx
xxx:
	bl	1f
1:	sub	r0,r0,1b-xxx
