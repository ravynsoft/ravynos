	.text
	adrp  x2, _GLOBAL_OFFSET_TABLE_
	ldr	x0, [x2, #:gotpage_lo14:globala]
	ldr	x0, [x2, #:gotpage_lo14:globalb]
	ldr	x0, [x2, #:gotpage_lo14:globalc]
