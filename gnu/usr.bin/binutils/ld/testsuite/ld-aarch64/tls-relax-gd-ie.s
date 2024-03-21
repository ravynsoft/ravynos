	.text
	adrp	x0, :tlsgd:var
	add	x0, x0, :tlsgd_lo12:var
	bl	__tls_get_addr
	nop
	ldr	w0, [x0]
