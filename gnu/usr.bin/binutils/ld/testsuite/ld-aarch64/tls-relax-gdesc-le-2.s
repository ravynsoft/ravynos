# Test TLS Desc to TLS LE relaxation when instructions are not consecutive.

	.section	.tdata
var:
	.word	2
	.text
	adrp	x0, :tlsdesc:var
	nop
	nop
	ldr	x1, [x0, #:tlsdesc_lo12:var]
	nop
	add	x0, x0, :tlsdesc_lo12:var
	nop
	nop
	nop
	.tlsdesccall	var
	blr	x1
	nop
	mrs	x1, tpidr_el0
	add	x0, x1, x0
	ldr	w0, [x0]
	.section	.tdata
