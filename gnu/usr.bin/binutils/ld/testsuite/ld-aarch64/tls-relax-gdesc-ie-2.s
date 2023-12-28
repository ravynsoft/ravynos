// Test TLS Desc to TLS IE relaxation when instructions are not consecutive.

	.text
	adrp	x0, :tlsdesc:var
	nop
	ldr	x1, [x0, #:tlsdesc_lo12:var]
	nop
	nop
	nop
	add	x0, x0, :tlsdesc_lo12:var
	nop
	nop
	.tlsdesccall	var
	blr	x1
	nop
	mrs	x1, tpidr_el0
	add	x0, x1, x0
	ldr	w0, [x0]
	.section	.tdata
