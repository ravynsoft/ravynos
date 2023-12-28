	.macro vmul_iter reg0 reg1 reg2 idx
	.irp op, vmul.i16 vmul.f16 vmul.i32 vmul.f32
		\op d\reg0, d\reg1, d\reg2[\idx]
		\op q\reg0, q\reg1, d\reg2[\idx]
	.endr
	.endm

	.macro vmul_acc_iter reg0 reg1 reg2 idx
	.irp op, vmla.i16 vmla.i32 vmla.f16 vmla.f32 vmls.i16 vmls.i32 vmls.f16 vmls.f32
		\op d\reg0, d\reg1, d\reg2[\idx]
		\op q\reg0, q\reg1, d\reg2[\idx]
	.endr
	.endm

	# There are two restriction on the scalar operand:
	#   * The scalar operand is restricted to D0-D7 if size is 16bit wide,
	#     or D0 - D15 otherwise.
	#   * The scalar index should within range, 0-3 if size is 16bit wide,
	#     0-1 if size is 32bit wide.
	vmul_iter 0 1 3 0
	vmul_iter 3 12 7 2
	vmul_iter 4 9 8 1
	vmul_iter 13 6 15 3
	vmul_acc_iter 2 7 1 0
	vmul_acc_iter 5 4 6 2
	vmul_acc_iter 4 13 10 1
	vmul_acc_iter 12 6 13 3
