	.text

bar:
	.space	0x10000

	.ent	foo
	.set	mips16
foo:
	dla	$2, . - 1
	nop
	ld	$2, . - 5
	nop
	dla	$2, . - 1 + 0x7c
	nop
	ld	$2, . - 5 + 0xf8
	nop
	dla	$2, . - 1 + 0x80
	ld	$2, . - 5 + 0x100
	dla	$2, . - 1 - 0x4
	ld	$2, . - 5 - 0x4
	dla	$2, . - 1 + 0x7fff
	ld	$2, . - 5 + 0x7fff
	dla	$2, . - 1 - 0x8000
	ld	$2, . - 5 - 0x8000

	# Pad these to retain alignment between MIPS16 and MIPS16e2 code.
0:	dla	$2, . - 1 + 0x8000
1:	.insn
	.fill	(12 - (1b - 0b)) / 2, 2, 0x6500
0:	ld	$2, . - 5 + 0x8000
1:	.insn
	.fill	(12 - (1b - 0b)) / 2, 2, 0x6500
0:	dla	$2, . - 1 - 0x8001
1:	.insn
	.fill	(12 - (1b - 0b)) / 2, 2, 0x6500
0:	ld	$2, . - 5 - 0x8001
1:	.insn
	.fill	(12 - (1b - 0b)) / 2, 2, 0x6500

	nop
	.set	nomips16
	.end	foo

baz:
	.align	7, 0
	.space	0xff00

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	4, 0
	.space	16
