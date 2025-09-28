	.text
	adrp	x0, :tlsdesc:var
	ldr	x17, [x0, #:tlsdesc_lo12:var]
	add	x0, x0, :tlsdesc_lo12:var
	.tlsdesccall	var
	blr	x1
	mrs	x1, tpidr_el0
	add	x0, x1, x0
	ldr	w0, [x0]
