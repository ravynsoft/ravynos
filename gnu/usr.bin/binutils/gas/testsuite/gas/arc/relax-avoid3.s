test:
	add	r0,r0,r0
	.weak	test
main:
	b	test
