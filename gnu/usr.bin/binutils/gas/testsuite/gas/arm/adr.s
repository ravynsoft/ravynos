	@ test ADR pseudo-op
	.text
	.global foo
foo:
	adrhi	pc, . - 0x2ffffff8
