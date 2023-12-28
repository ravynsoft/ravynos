/* Test file for ARMv8.3 complex arithmetics instructions.  */
	.text

	.macro three_same op, sz
	.irp rot, 0, 90, 180, 270
	.irp d, 1.\sz, 2.\sz, 5.\sz, 13.\sz, 27.\sz
	.irp m, 2.\sz, 3.\sz, 5.\sz, 14.\sz, 31.\sz
	.irp n, 3.\sz, 4.\sz, 6.\sz, 15.\sz, 30.\sz
		\op v\d, v\m, v\n, #\rot
	.endr
	.endr
	.endr
	.endr
	.endm

	.macro three_element op, sz1, sz2, idx
	.irp rot, 0, 90, 180, 270
	.irp d, 1.\sz1, 2.\sz1, 5.\sz1, 13.\sz1, 27.\sz1
	.irp m, 2.\sz1, 3.\sz1, 5.\sz1, 14.\sz1, 31.\sz1
	.irp n, 3.\sz2, 4.\sz2, 6.\sz2, 15.\sz2, 30.\sz2
		\op v\d, v\m, v\n[\idx], #\rot
	.endr
	.endr
	.endr
	.endr
	.endm

	.macro three_same_rot op, sz
	.irp rot, 90, 270
	.irp d, 1.\sz, 2.\sz, 5.\sz, 13.\sz, 27.\sz
	.irp m, 2.\sz, 3.\sz, 5.\sz, 14.\sz, 31.\sz
	.irp n, 3.\sz, 4.\sz, 6.\sz, 15.\sz, 30.\sz
		\op v\d, v\m, v\n, #\rot
	.endr
	.endr
	.endr
	.endr
	.endm

	/* Three-same operands FCMLA.  */
	three_same fcmla, 2d
	three_same fcmla, 2s
	three_same fcmla, 4s
	three_same fcmla, 4h
	three_same fcmla, 8h

	/* Indexed element FCMLA.  */
	three_element fcmla, 4s, s, 0
	three_element fcmla, 4s, s, 1

	three_element fcmla, 4h, h, 0
	three_element fcmla, 4h, h, 1

	three_element fcmla, 8h, h, 0
	three_element fcmla, 8h, h, 1
	three_element fcmla, 8h, h, 2
	three_element fcmla, 8h, h, 3

	/* Three-same operands FADD.  */
	three_same_rot fcadd, 2d
	three_same_rot fcadd, 2s
	three_same_rot fcadd, 4s
	three_same_rot fcadd, 4h
	three_same_rot fcadd, 8h
