	.global var
	.global dummy
	.text
test:
	ldr x1, .Lgot
	adr x2, .Lgot
	add x1, x1, x2

	movk x0, #:tlsgd_g0_nc:var
	movk x0, #:tlsgd_g0_nc:dummy
	add x0, x1, x0
	bl   __tls_get_addr
	nop

.Lgot: .xword _GLOBAL_OFFSET_TABLE_ - .
