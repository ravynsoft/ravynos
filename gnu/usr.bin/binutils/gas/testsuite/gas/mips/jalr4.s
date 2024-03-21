	.abicalls
	.text

	.align	2
	.globl	foo
	.ent	foo
foo:
	.reloc	1f, R_MIPS_JALR, bar0
1:	jalr	$25
	.reloc	1f, R_MIPS_JALR, bar0
1:	jalr	$0, $25
	.reloc	1f, R_MIPS_JALR, bar0
1:	jr	$25
	.reloc	1f, R_MIPS_JALR, bar1
1:	jalr	$25
	.reloc	1f, R_MIPS_JALR, bar1
1:	jalr	$0, $25
	.reloc	1f, R_MIPS_JALR, bar1
1:	jr	$25
	.reloc	1f, R_MIPS_JALR, bar2
1:	jalr	$25
	.reloc	1f, R_MIPS_JALR, bar2
1:	jalr	$0, $25
	.reloc	1f, R_MIPS_JALR, bar2
1:	jr	$25
	.end	foo

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	4, 0
	.space	16

	.align	2
	.globl	bar0
	.ent	bar0
bar0:
	.insn
	.end	bar0

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	4, 0
	.space	16

	.align	2
	.globl	bar1
	.ent	bar1
	.space	2
bar1:
	.insn
	.end	bar1

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	4, 0
	.space	16

	.set	mips64r2
	.set	mips16
	.align	2
	.globl	bar2
	.ent	bar2
	.byte	0
bar2:
	.insn
	.end	bar2
	.set	nomips16
	.set	mips0

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	4, 0
	.space	16
