# Test TLS IE to TLS LE relaxation when using arbitrary registers.

	.section	.tdata
var:
	.word	2
	.text
	mrs	x2, tpidr_el0
	adrp	x15, :gottprel:var
	ldr	x15, [x15, #:gottprel_lo12:var]
	add	x15, x2, x15
	ldr	w0, [x15]
	.section	.tdata
