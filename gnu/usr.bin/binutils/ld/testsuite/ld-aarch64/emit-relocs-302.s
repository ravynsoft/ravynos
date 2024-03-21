	.text
	ldr	x1, .Lgot
	adr	x2, .Lgot
	add	x1, x2, x1

	movz	x0, #:gotoff_g1:globala
	movz	x0, #:gotoff_g1:globalb
	movz	x0, #:gotoff_g1:globalc
	ldr	x0, [x1, x0]

.Lgot:
	.dword _GLOBAL_OFFSET_TABLE_ - .
