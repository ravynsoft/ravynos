	# Test R_SPARC_PC22 and R_SPARC_PC10 relocations
	.data
sym:	.word	0
	.text
	sethi	%pc22(sym), %o1
	or	%o1, %pc10(sym), %o1
