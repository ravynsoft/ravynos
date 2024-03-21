	.section	.tdata
var:
	.word	2

	.text
test:
	ldr	x1, .Lgot
	adr	x2, .Lgot
	add	x1, x2, x1

	movz	x0, #:tlsgd_g1:var
	movk	x0, #:tlsgd_g0_nc:var
	add	x0, x1, x0
	bl	__tls_get_addr
	nop
	ldr	w0, [x0]

.Lgot:
	.dword _GLOBAL_OFFSET_TABLE_ - .
