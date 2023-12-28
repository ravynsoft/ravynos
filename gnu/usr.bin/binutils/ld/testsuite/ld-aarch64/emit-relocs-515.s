	.global var
	.text
test:
	ldr x1, .Lgot
	adr x2, .Lgot
	add x1, x1, x2

	movz x0, #:tlsgd_g1:var
	add x0, x1, x0
	bl   __tls_get_addr
	nop

.Lgot: .xword _GLOBAL_OFFSET_TABLE_ - .
