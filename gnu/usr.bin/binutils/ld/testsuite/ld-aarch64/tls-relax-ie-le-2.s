# Test TLS IE to TLS LE relaxation when instructions are not consecutive.

	.section	.tdata
var:
	.word	2
	.text
	mrs	x1, tpidr_el0
	nop
	nop
	adrp	x0, :gottprel:var
	nop
	nop
	nop
	ldr	x0, [x0, #:gottprel_lo12:var]
	nop
	add	x0, x1, x0
	nop
	nop
	ldr	w0, [x0]
	.section	.tdata
