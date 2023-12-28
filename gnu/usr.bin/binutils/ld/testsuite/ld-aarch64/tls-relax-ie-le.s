	.section	.tdata
var:
	.word	2
	.text
	mrs	x1, tpidr_el0
	adrp	x0, :gottprel:var
	ldr	x0, [x0, #:gottprel_lo12:var]
	add	x0, x1, x0
	ldr	w0, [x0]
