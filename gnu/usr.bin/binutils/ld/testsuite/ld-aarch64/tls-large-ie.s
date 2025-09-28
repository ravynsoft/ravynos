	.global var
	.text
test:
	ldr x1, .Lgot
	adr x2, .Lgot
	add x1, x1, x2

	mrs x2, tpidr_el0
	movz x0, #:gottprel_g1:var
	movk x0, #:gottprel_g0_nc:var
	ldr x0, [x1, x0]
	add x0, x0, x2
	nop

.Lgot: .xword _GLOBAL_OFFSET_TABLE_ - .
