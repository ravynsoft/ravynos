# Source file used to test the miscellaneous instructions.

foo:
L1:
	jmp	r10
	jmp	r10.w0
	jmp	0x100

L2:
	jal	r22, r10.w2
	jal	r23, 0
	jal	r23.w1, 0x3fffc

	# relative branches - forward jump
L3:
	qbgt	L5, r23, 0
	qbge	L5, r23.b2, 255
	qblt	L5, r22.w1, r0.b1
	qble	L5, r0.b0, r1.b1
	qbeq	L5, r1.b2, r3.b0
	qbne	L5, r21, r22
	qba	L5

	qbbs	L5, r12, r13
	qbbs	L5, r12, 5
	qbbc	L5, r12, r13
	qbbc	L5, r12, 5

	# relative branches - backward jump
L4:
	qbgt	L2, r23, 0
	qbge	L2, r23.b2, 255
	qblt	L2, r22.w1, r0.b1
	qble	L2, r0.b0, r1.b1
	qbeq	L2, r1.b2, r3.b0
	qbne	L2, r21, r22
	qba	L2

L5:
	qbbs	L2, r12, r13
	qbbs	L2, r12, 5
	qbbc	L2, r12, r13
