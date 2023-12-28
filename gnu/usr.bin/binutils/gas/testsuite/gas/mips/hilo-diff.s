# Source code used to test %hi/%lo operators with constants worked out
# as a difference of local symbols, both previously defined and forward
# references.

	.text
foo:
	li	$4, %hi(2f - 0f)
	sll	$4, 16
	addiu	$4, %lo(2f - 0f)
	li	$5, %hi(0f - 3f)
	sll	$5, 16
	addiu	$5, %lo(0f - 3f)
0:
	li	$4, %hi(2f - 0b)
	sll	$4, 16
	addiu	$4, %lo(2f - 0b)
	li	$5, %hi(0b - 3f)
	sll	$5, 16
	addiu	$5, %lo(0b - 3f)
1:
	.insn
	.fill	0x7ffc - (1b - 0b)
2:
	.insn
	.fill	0x8000 - (2b - 0b)
3:
	li	$4, %hi(2b - 0b)
	sll	$4, 16
	addiu	$4, %lo(2b - 0b)
	li	$5, %hi(0b - 3b)
	sll	$5, 16
	addiu	$5, %lo(0b - 3b)

	li	$4, %hi(2f - 0f)
	sll	$4, 16
	addiu	$4, %lo(2f - 0f)
	li	$5, %hi(0f - 3f)
	sll	$5, 16
	addiu	$5, %lo(0f - 3f)
0:
	li	$4, %hi(2f - 0b)
	sll	$4, 16
	addiu	$4, %lo(2f - 0b)
	li	$5, %hi(0b - 3f)
	sll	$5, 16
	addiu	$5, %lo(0b - 3f)
1:
	.insn
	.fill	0x8000 - (1b - 0b)
2:
	.insn
	.fill	0x8004 - (2b - 0b)
3:
	li	$4, %hi(2b - 0b)
	sll	$4, 16
	addiu	$4, %lo(2b - 0b)
	li	$5, %hi(0b - 3b)
	sll	$5, 16
	addiu	$5, %lo(0b - 3b)

	li	$4, %hi(2f - 0f)
	sll	$4, 16
	addiu	$4, %lo(2f - 0f)
	li	$5, %hi(0f - 3f)
	sll	$5, 16
	addiu	$5, %lo(0f - 3f)
0:
	li	$4, %hi(2f - 0b)
	sll	$4, 16
	addiu	$4, %lo(2f - 0b)
	li	$5, %hi(0b - 3f)
	sll	$5, 16
	addiu	$5, %lo(0b - 3f)
1:
	.insn
	.fill	0x11ffdc - (1b - 0b)
2:
	.insn
	.fill	0
3:
	li	$4, %hi(2b - 0b)
	sll	$4, 16
	addiu	$4, %lo(2b - 0b)
	li	$5, %hi(0b - 3b)
	sll	$5, 16
	addiu	$5, %lo(0b - 3b)

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	4, 0
	.space	16
