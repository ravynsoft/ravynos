	.text
errata:
	.cpu	msp430
	# CPU4: PUSH #4/#8 has to be encoded using the long form
	push	#4
	push	#8

	# CPU8: Do not use odd offsets with the stack pointer
	.set fred,3
	.set bert,1
	mov.w	#4, 7(sp)
	mov	3(r1), 5(r0)
	mov.b	@r10+,fred-bert+1(sp)
	add.w	#1,1(sp)
	add.w	#7,-1(sp)

	# CPU11: The SR flags can be left in a bogus state after writing to the PC
	add.w	 #3, pc
	and	 #1, pc
	bit	 #1, pc
	dadd	 #1, pc
	inc	 pc
	incd	 pc
	sub	 #1, pc
	subc	 #1, pc
	xor	 #1, pc

	#CPU12: A CMP or BIT instruction with the PC as the second operand may
	# not execute the instruction after it.
	cmp   	 &200, PC
	bit	 r1, pc

	#CPU13: Arithmetic operations with SR as the destination do not work.
	add	#3, sr
	adc	sr
	addc	#3, sr
	and	#3, sr
	dadd	#3, sr
	dec	sr
	decd	sr
	inc	sr
	incd	sr
	inv	sr
	rla	sr
	rlc	sr
	rra	sr
	rrc	sr
	sbc	sr
	sub	#3, sr
	subc	#3, sr
	sxt	sr
	xor	#3, sr

	#CPU19: Instructions that sets CPUOFF must be followed by a NOP
	bis	#0x10, r2
	mov	#0x10, r2
	xor	#0x10, r2
	nop
	