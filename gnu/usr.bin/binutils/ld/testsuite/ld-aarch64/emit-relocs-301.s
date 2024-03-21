	.text
	ldr	x1, .Lgot
	adr	x2, .Lgot
	add	x1, x2, x1

	movk	x0, #:gotoff_g0_nc:globala
	movk	x0, #:gotoff_g0_nc:globalb
	movk	x0, #:gotoff_g0_nc:globalc
	ldr	x0, [x1, x0]

.Lgot:
	.dword _GLOBAL_OFFSET_TABLE_ - .
