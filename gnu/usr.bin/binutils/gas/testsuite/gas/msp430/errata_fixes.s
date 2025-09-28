	.text
errata:
	.cpu	msp430
	# CPU4: PUSH #4/#8 has to be encoded using the long form
	push	#4
	push	#8

	# CPU11: The SR flags can be left in a bogus state after writing to the PC
	# Instructions that do not set the SR flags are unaffected.
	bic	 #1, pc
	bis	 #1, pc
	mov	 #1, pc
	
	#CPU12: A CMP or BIT instruction with the PC as the second operand may
	# not execute the instruction after it - so a NOP must be inserted.
	cmp   	 &200, PC
	bit	 r1, pc

	#CPU19: Instructions that sets CPUOFF must be followed by a NOP
	bis	#0x10, r2
	mov	#0x10, r2
	xor	#0x10, r2
	nop
	