# Source file used to test the ALU class of instructions.

foo:
	# Test various addressing modes
	add	fp, fp, fp
	add	fp, fp, 0xff
	add	fp, fp, 0
	add	fp, fp, 0
	add	fp.b1, fp.w1, 0
	add	r1.b1, r2.b2, r3.b3
	adc	r1.b1, r2.b2, r3.b3
	adc	r1.b1, r2.b2, 101-2

	# Test ALU opcodes
	add	r0, r0, r0
	adc	r0, r0, r0
	sub	r1, r31, 10
	suc	r1, r31, 10
	lsl	r31, r31, 10
	lsr	r31, r31, 10
	rsb	r16, r16.b3, 10
	rsc	r16, r16.b3, 10
	and	r1.w1, r1.b3, 0xaa
	or	r1.w1, r1.b3, 0xaa
	xor	r1.w1, r1.b3, 0xaa
	not	r2, r1
	min	r1, r1, r2
	max	r1, r2, r3.w2
	clr	r1, r2, r3.w2
	set	r1, r2, 12
