	.text
	.align	2
	.global	foo
foo:
	mul	r0, r0, r0
	mla	r0, r0, r1, r2
	mls	r0, r0, r1, r2
	umull	r0, r1, r0, r1
	smlalbb	r0, r1, r0, r1
	bx	lr
