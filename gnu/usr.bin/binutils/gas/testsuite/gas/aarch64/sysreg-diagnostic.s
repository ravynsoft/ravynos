.text
	msr     dbgdtrtx_el0, x3
	msr     dbgdtrrx_el0, x3
	mrs     x3, dbgdtrrx_el0
	mrs     x3, dbgdtrtx_el0
	msr     midr_el1, x3
	msr	id_aa64isar2_el1, x0
