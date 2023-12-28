	.text
test:
	ldr x1, .Lgot
	adr x2, .Lgot
	add x18, x1, x2

	movz x0, #:tlsdesc_off_g1:var
	movk x0, #:tlsdesc_off_g0_nc:var
	.tlsdescldr var
	ldr x1, [x18, x0]
	.tlsdescadd var
	add x0, x18, x0
	.tlsdesccall var
	blr x1

.Lgot: .xword _GLOBAL_OFFSET_TABLE_ - .
