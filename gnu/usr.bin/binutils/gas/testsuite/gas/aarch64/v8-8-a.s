	.arch armv8.8-a

	ld64b x0, [x1]
	dsb oshnxs
	msr hcrx_el2, x0
