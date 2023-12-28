# Source file used to test microMIPS instruction size overrides (#1).

	.text
foo:
# Smoke-test a trivial case.
	nop
	nop16
	nop32

# Test ALU operations.
	addu	$2, $4
	addu16	$2, $4
	addu32	$2, $4
	addu	$12, $14
	addu32	$12, $14
	add.ps	$f2, $f4
	add32.ps $f2, $f4
	addiusp	256
	addiusp16 256

# Test jumps and branches.
	jalr	$4
	jalr16	$4
	jalr32	$4
	jalr	$24
	jalr16	$24
	jalr32	$24
	jalr	$31,$5
	jalr16	$31,$5
	jalr32	$31,$5
	jalr	$31,$25
	jalr16	$31,$25
	jalr32	$31,$25
	jalr	$30,$26
	jalr32	$30,$26
	nop
	b	bar
	nop
	b16	bar
	nop
	b32	bar
	nop
	beqz	$7, bar
	nop
	beqz16	$7, bar
	nop
	beqz32	$7, bar
	nop
	beqz	$27, bar
	nop
	beqz32	$27, bar

# Test branch delay slots.
	.set	noreorder
	bltzal	$2, bar
	 addu	$16, $17
	bltzal	$2, bar
	 addu16	$16, $17
	bltzal	$2, bar
	 addu32	$16, $17
	bltzals	$2, bar
	 addu	$16, $17
	bltzals	$2, bar
	 addu16	$16, $17
	bltzals	$2, bar
	 addu32	$16, $17
	bltzal	$2, bar
	 add.ps	$f2, $f4
	bltzal	$2, bar
	 add32.ps $f2, $f4
	bltzals	$2, bar
	 add.ps	$f2, $f4
	bltzals	$2, bar
	 add32.ps $f2, $f4
	bltzal	$2, bar
	 addiusp 256
	bltzal	$2, bar
	 addiusp16 256
	bltzals	$2, bar
	 addiusp 256
	bltzals	$2, bar
	 addiusp16 256
	.set	reorder

# Test macro delay slots.
	.set	noreorder
	bltzall	$2, bar
	 addu	$16, $17
	bltzall	$2, bar
	 addu16	$16, $17
	bltzall	$2, bar
	 addu32	$16, $17
	bltzall	$2, bar
	 add.ps	$f2, $f4
	bltzall	$2, bar
	 add32.ps $f2, $f4
	bltzall	$2, bar
	 addiusp 256
	bltzall	$2, bar
	 addiusp16 256
	.set	reorder

# Test shift instructions to complement 64-bit tests.
	sll	$2, $3, 5
	sll16	$2, $3, 5
	sll32	$2, $3, 5
	sll	$2, $3, 13
	sll32	$2, $3, 13
	sll	$10, $11, 5
	sll32	$10, $11, 5

# Test 64-bit instructions.
	dsll	$2, $3, 5
	dsll32	$2, $3, 5			# No way to force 32-bit DSLL.
	dsll3232 $2, $3, 5
	dsll	$2, $3, 13
	dsll32	$2, $3, 13			# No way to force 32-bit DSLL.
	dsll3232 $2, $3, 13
	dsll	$10, $11, 5
	dsll32	$10, $11, 5			# No way to force 32-bit DSLL.
	dsll3232 $10, $11, 5

# Force at least 8 (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	2
	.space	8
