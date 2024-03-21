	.global	v1
	.global	v2
	.section	.tdata,"awT",%progbits
v1:
	.word	1

	.text

# This GD access does not relax. It consumes a double GOT slot.

	adrp	x0, :tlsgd:v2
	add	x0, x0, :tlsgd_lo12:v2
	bl	__tls_get_addr
	nop

# Test the combination of a TLSDESC-GD and IE access to the same
# symbol. We expect the TLSDESC-GD to relax to IE.

	adrp	x0, :tlsdesc:v1
	ldr	x1, [x0, #:tlsdesc_lo12:v1]
	add	x0, x0, :tlsdesc_lo12:v1
	.tlsdesccall	v1
	blr	x1
	mrs	x1, tpidr_el0
	add	x0, x1, x0

	mrs	x2, tpidr_el0
	adrp	x0, :gottprel:v1
	ldr	x0, [x0, #:gottprel_lo12:v1]
	add	x0, x2, x0
	ldr	w0, [x0]
	add	w0, w1, w0
