# relocs against undefined weak symbols should not be treated as
# overflowing

	.module mips64r6
	.globl	start
	.type	start, @function
	.weak	foo
start:
	.set noreorder
	.set mips64r6
	beqzc	$2, foo
	nop 
	bnezc	$2, foo
	lwpc	$2, foo
	ldpc	$2, foo
	bc	foo

	b	foo
	nop
	bal	foo
	lui	$4, %gp_rel(foo)

	jal	foo
	nop
	j	foo
	nop

	.set mips32r2
	.set micromips
micro:
	beqz16	$4, foo
	nop
	b16	foo
	nop
	b	foo
	nop
	bal	foo
	nop

	jal	foo
	nop
	j	foo
	nop

	.set nomicromips
	.set mips16
mips16:
	b	foo

	jal	foo
	nop
