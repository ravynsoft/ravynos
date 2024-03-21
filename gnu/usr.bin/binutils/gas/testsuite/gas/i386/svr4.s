	.text

	.if 1 / 2
	.else
	.error
	.endif

	.if 1 \/ 2
	.error
	.endif

	.if 4 \% 2
	.error
	.endif

	.if 1 \* 0
	.error
	.endif

svr4:
	mov	$(15 \/ 2), %al
	mov	$(15 \% 2), %al
	mov	$(15 \* 2), %al

	.byte	0xb0, 17 \/ 3
	.byte	0xb0, 17 \% 3
	.byte	0xb0, 17 \* 3

	.equiv	a, 19 \/ 4
	.equiv	b, 19 \% 4
	.equiv	c, 19 \* 4
