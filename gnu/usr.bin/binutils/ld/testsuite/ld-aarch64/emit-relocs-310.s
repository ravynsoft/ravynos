	.text
	ldr	x1, .Lgot
	adr	x2, .Lgot
	add	x1, x2, x1

	ldr	x0, [x1, #:gotoff_lo15:globala]
	ldr	x0, [x1, #:gotoff_lo15:globalb]
	ldr	x0, [x1, #:gotoff_lo15:globalc]

.Lgot:
	.dword _GLOBAL_OFFSET_TABLE_ - .
