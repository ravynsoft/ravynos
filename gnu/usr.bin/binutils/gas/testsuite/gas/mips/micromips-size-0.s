# Source file used to test the microMIPS instruction size overrides (#0).

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
	addu16	$12, $14
	addu32	$12, $14
	add.ps	$f2, $f4
	add16.ps $f2, $f4
	add32.ps $f2, $f4
	addiusp	256
	addiusp16 256
	addiusp32 256

# Test jumps and branches.
	jar	$23
	jar16	$23
	jar32	$23
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
	jalr16	$30,$26
	jalr32	$30,$26
	b	bar
	b16	bar
	b32	bar
	beqz	$7, bar
	beqz16	$7, bar
	beqz32	$7, bar
	beqz	$27, bar
	beqz16	$27, bar
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
	 add16.ps $f2, $f4
	bltzal	$2, bar
	 add32.ps $f2, $f4
	bltzals	$2, bar
	 add.ps	$f2, $f4
	bltzals	$2, bar
	 add16.ps $f2, $f4
	bltzals	$2, bar
	 add32.ps $f2, $f4
	bltzal	$2, bar
	 addiusp 256
	bltzal	$2, bar
	 addiusp16 256
	bltzal	$2, bar
	 addiusp32 256
	bltzals	$2, bar
	 addiusp 256
	bltzals	$2, bar
	 addiusp16 256
	bltzals	$2, bar
	 addiusp32 256
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
	 add16.ps $f2, $f4
	bltzall	$2, bar
	 add32.ps $f2, $f4
	bltzall	$2, bar
	 addiusp 256
	bltzall	$2, bar
	 addiusp16 256
	bltzall	$2, bar
	 addiusp32 256
	.set	reorder

# Test shift instructions to complement 64-bit tests.
	sll	$2, $3, 5
	sll16	$2, $3, 5
	sll32	$2, $3, 5
	sll	$2, $3, 13
	sll16	$2, $3, 13
	sll32	$2, $3, 13
	sll	$10, $11, 5
	sll16	$10, $11, 5
	sll32	$10, $11, 5

# Test 64-bit instructions.
	dsll	$2, $3, 5
	dsll16	$2, $3, 5
	dsll32	$2, $3, 5			# No way to force 32-bit DSLL.
	dsll3216 $2, $3, 5
	dsll3232 $2, $3, 5
	dsll	$2, $3, 13
	dsll16	$2, $3, 13
	dsll32	$2, $3, 13			# No way to force 32-bit DSLL.
	dsll3216 $2, $3, 13
	dsll3232 $2, $3, 13
	dsll	$10, $11, 5
	dsll16	$10, $11, 5
	dsll32	$10, $11, 5			# No way to force 32-bit DSLL.
	dsll3216 $10, $11, 5
	dsll3232 $10, $11, 5

# Test out-of-range mapped constants
	addiu16 $2, $4, 4		# OK
	addiu16 $2, $4, 5		# error
	addiu16 $2, $4, 7		# error
	addiu16 $2, $4, 8		# OK
	andi16 $2, $4, 4		# OK
	andi16 $2, $4, 5		# error
	andi16 $2, $4, 7		# OK
	andi16 $2, $4, 8		# OK

# Test invalid ADDIUSP
	addiusp16 4			# error
	addiusp16 7			# error
	addiusp16 8			# OK
	addiusp16 10			# error
	addiusp16 12			# OK
	addiusp16 1028			# OK
	addiusp16 1032			# error
	addiusp16 -1032			# OK
	addiusp16 -1036			# error

# Force at least 8 (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	2
	.space	8
