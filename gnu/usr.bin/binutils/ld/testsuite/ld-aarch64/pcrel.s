	.hidden global_b
	.text
	.align 2
main:
	# R_AARCH64_ADR_PREL_PG_HI21
	# R_AARCH64_ADR_PREL_PG_HI21_NC
	# R_AARCH64_ADR_LO_21
	adrp	x0, :pg_hi21:global_a
	adrp	x1, :pg_hi21_nc:global_a
	adr	x2, global_a

	#R_AARCH64_LD_PREL_LO19
	ldr	x3, global_a

	# R_AARCH64_PREL16
	# R_AARCH64_PREL32
	# R_AARCH64_PREL64
	.hword	global_a - .
	.word	global_a - .
	.xword	global_a - .

	# Defined global symbol may bind externally because of copy relocation,
	# while defined hidden symbol binds locally.  LD should be able to
	# differenciate this.
	adrp	x0, :pg_hi21:global_b
	.xword	global_b - .
