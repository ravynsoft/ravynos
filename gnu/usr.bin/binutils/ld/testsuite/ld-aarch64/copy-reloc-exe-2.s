	# expect copy relocation for all these scenarios.
	.global	p
	.global	q
	.global	r
	.section	.data.rel.ro,"aw",%progbits
	.align	3
	.type	p, %object
	.size	p, 8
p:
	.xword	global_a

	.type	q, %object
	.size	q, 8
q:
	.xword	global_b

	.type	r, %object
	.size	r, 8
r:
	# Any pc-rel relocation as no dynamic linker support on AArch64.
	.xword	global_c - .

	.text
	.global main
main:
	# Symbols are referenced by any other relocation against read-only
	# section.
	movz x0, :abs_g0_nc:global_a
	adrp x1, global_b
	# pc-rel.
	adrp x2, global_d
	add x2, x2, #:lo12:global_c
