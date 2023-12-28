	.include "advsimd-armv8_3.s"

	.macro three_same_no_rot op, sz
	.irp d, 1.\sz, 2.\sz, 5.\sz, 13.\sz, 27.\sz
	.irp m, 2.\sz, 3.\sz, 5.\sz, 14.\sz, 31.\sz
	.irp n, 3.\sz, 4.\sz, 6.\sz, 15.\sz, 30.\sz
		\op v\d, v\m, v\n
	.endr
	.endr
	.endr
	.endm

	three_same_no_rot fadd, 2d
	three_same_no_rot fadd, 2s
	three_same_no_rot fadd, 4h
	three_same_no_rot fadd, 8h
