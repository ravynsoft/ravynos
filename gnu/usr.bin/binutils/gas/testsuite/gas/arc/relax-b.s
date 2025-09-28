	.text
	nop_s
	.align 4
foo:
	add	r0,r0,r0

	.align 4
bar:
	bl	@foo
	add	r1,r1,r1
	b	@foo
