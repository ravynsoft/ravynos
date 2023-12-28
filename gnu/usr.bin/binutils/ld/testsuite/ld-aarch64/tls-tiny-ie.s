	.global	v1
	.section	.tdata,"awT",%progbits
v1:
	.word	1

	.text

# Test tiny TLS IE.
	mrs	x2, tpidr_el0
	ldr	x0, #:gottprel:v1
	add	x0, x2, x0
	ldr	w0, [x0]
