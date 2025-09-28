# Source file used to test microMIPS branch delay slots.

	.text
foo:
	.set	noreorder
	bltzal	$2, .
	 li	$2, -1
	bltzal	$2, .
	 li	$2, 0x7fff
	bltzal	$2, .
	 li	$2, 0xffff
	bltzal	$2, .
	 li	$2, 0x10000
	bltzals	$2, .
	 li	$2, -1
	bltzals	$2, .
	 li	$2, 0x7fff
	bltzals	$2, .
	 li	$2, 0xffff
	bltzals	$2, .
	 li	$2, 0x10000
	bltzall	$2, .
	 li	$2, -1
	bltzall	$2, .
	 li	$2, 0x7fff
	bltzall	$2, .
	 li	$2, 0xffff
	bltzall	$2, .
	 li	$2, 0x10000

	bltzal	$2, .
	 addiu	$2, $29, -1
	bltzal	$2, .
	 addiu	$2, $29, 8
	bltzal	$2, .
	 addiu	$2, $29, 256
	bltzal	$2, .
	 addiu	$2, $29, 0x7fff
	bltzals	$2, .
	 addiu	$2, $29, -1
	bltzals	$2, .
	 addiu	$2, $29, 8
	bltzals	$2, .
	 addiu	$2, $29, 256
	bltzals	$2, .
	 addiu	$2, $29, 0x7fff
	bltzall	$2, .
	 addiu	$2, $29, -1
	bltzall	$2, .
	 addiu	$2, $29, 8
	bltzall	$2, .
	 addiu	$2, $29, 256
	bltzall	$2, .
	 addiu	$2, $29, 0x7fff

	bltzal	$2, .
	 addiu	$29, $29, -1
	bltzal	$2, .
	 addiu	$29, $29, 8
	bltzal	$2, .
	 addiu	$29, $29, 256
	bltzal	$2, .
	 addiu	$29, $29, 0x7fff
	bltzals	$2, .
	 addiu	$29, $29, -1
	bltzals	$2, .
	 addiu	$29, $29, 8
	bltzals	$2, .
	 addiu	$29, $29, 256
	bltzals	$2, .
	 addiu	$29, $29, 0x7fff
	bltzall	$2, .
	 addiu	$29, $29, -1
	bltzall	$2, .
	 addiu	$29, $29, 8
	bltzall	$2, .
	 addiu	$29, $29, 256
	bltzall	$2, .
	 addiu	$29, $29, 0x7fff

	bltzal	$2, .
	 addu	$2, $29, -1
	bltzal	$2, .
	 addu	$2, $29, 8
	bltzal	$2, .
	 addu	$2, $29, 256
	bltzal	$2, .
	 addu	$2, $29, 0x7fff
	bltzal	$2, .
	 addu	$2, $29, 0x10000
	bltzals	$2, .
	 addu	$2, $29, -1
	bltzals	$2, .
	 addu	$2, $29, 8
	bltzals	$2, .
	 addu	$2, $29, 256
	bltzals	$2, .
	 addu	$2, $29, 0x7fff
	bltzals	$2, .
	 addu	$2, $29, 0x10000
	bltzall	$2, .
	 addu	$2, $29, -1
	bltzall	$2, .
	 addu	$2, $29, 8
	bltzall	$2, .
	 addu	$2, $29, 256
	bltzall	$2, .
	 addu	$2, $29, 0x7fff
	bltzall	$2, .
	 addu	$2, $29, 0x10000

	bltzal	$2, .
	 addu	$29, $29, -1
	bltzal	$2, .
	 addu	$29, $29, 8
	bltzal	$2, .
	 addu	$29, $29, 256
	bltzal	$2, .
	 addu	$29, $29, 0x7fff
	bltzal	$2, .
	 addu	$29, $29, 0x10000
	bltzals	$2, .
	 addu	$29, $29, -1
	bltzals	$2, .
	 addu	$29, $29, 8
	bltzals	$2, .
	 addu	$29, $29, 256
	bltzals	$2, .
	 addu	$29, $29, 0x7fff
	bltzals	$2, .
	 addu	$29, $29, 0x10000
	bltzall	$2, .
	 addu	$29, $29, -1
	bltzall	$2, .
	 addu	$29, $29, 8
	bltzall	$2, .
	 addu	$29, $29, 256
	bltzall	$2, .
	 addu	$29, $29, 0x7fff
	bltzall	$2, .
	 addu	$29, $29, 0x10000
	.set	reorder

# Force at least 8 (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	2
	.space	8
