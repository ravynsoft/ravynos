	.text

	.set	$bar, bar

	.globl	foo
	.ent	foo
foo:
	.reloc	1f, R_MIPS_JALR, $bar
1:	jalr	$25
	.reloc	1f, R_MIPS_JALR, $bar
1:	jr	$25
	.end	foo

	.ent	bar
bar:
	j	$31
	.end	bar

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	4, 0
	.space	16
