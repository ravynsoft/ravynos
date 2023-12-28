# Test if all fpu ops are correctly disassembled as they share the
# same opcode space with FPX instructions.
	fcvt32		r0,r2,r4
	fcvt32_64	r0,r2,r4
	fcvt64		r0,r2,r4
	fcvt64_32	r0,r2,r4
	fdadd		r0,r2,r4
	fdcmp		r0,r2
	fdcmpf		r0,r2
	fddiv		r0,r2,r4
	fdmadd		r0,r2,r4
	fdmsub		r0,r2,r4
	fdmul		r0,r2,r4
	fdsqrt		r0,r2
	fdsub		r0,r2,r4
	fsadd		r0,r2,r4
	fscmp		r0,r2
	fscmpf		r0,r2
	fsdiv		r0,r2,r4
	fsmadd		r0,r2,r4
	fsmsub		r0,r2,r4
	fsmul		r0,r2,r4
	fssqrt		r0,r2
	fssub		r0,r2,r4
