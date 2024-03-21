# Source file used to test the load/store instructions.

foo:
	# immediate load
	ldi	r16, 0
	ldi	r16, 0xffff
	ldi	r16, 511

	# load
	lbbo	&r0.b1, r30, r1.b3, 1
	lbbo	r0.b2, r30, r1.b2, 124
	lbbo	r0.b3, r30, 255, 1
	lbbo	&r0, r30, 1, 2
	lbbo	r0, r30, 0, 0x55
	lbbo	r18, r25, r1.w1, r0.b0
	lbbo	r18, r25, 101, r0.b1
	lbbo	r18, r25, r1, r0.b3

	# store
	sbbo	&r0.b1, r30, r1.b3, 1
	sbbo	r0.b2, r30, r1.b2, 124
	sbbo	r0.b3, r30, 255, 1
	sbbo	&r0, r30, 1, 2
	sbbo	r0, r30, 0, 0x55
	sbbo	r18, r25, r1, r0.b0
	sbbo	r18, r25, 101, r0.b1
	sbbo	r18, r25, r1, r0.b3

	# load with constant table address
	lbco	r10, 0, 5, 8
	lbco	r10, 1, r11.w1, 8
	lbco	r10, 31, 5, 8

	# store with constant table address
	sbco	r10, 0, 5, 8
	sbco	r10, 1, r11.w1, 8
	sbco	r10, 31, 5, 8
