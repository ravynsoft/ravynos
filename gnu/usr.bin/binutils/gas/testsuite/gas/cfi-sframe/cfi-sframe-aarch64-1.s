        .cfi_sections .sframe
	.cfi_startproc
	stp	x19, x20, [sp, -144]!
	.cfi_def_cfa_offset 144
	.cfi_offset 19, -144
	.cfi_offset 20, -136
	stp	x21, x22, [sp, 16]
	stp	x23, x24, [sp, 32]
	stp	x25, x26, [sp, 48]
	stp	x27, x28, [sp, 64]
	stp	d8, d9, [sp, 80]
	stp	d10, d11, [sp, 96]
	stp	d12, d13, [sp, 112]
	stp	d14, d15, [sp, 128]
	.cfi_offset 21, -128
	.cfi_offset 22, -120
	.cfi_offset 23, -112
	.cfi_offset 24, -104
	.cfi_offset 25, -96
	.cfi_offset 26, -88
	.cfi_offset 27, -80
	.cfi_offset 28, -72
	.cfi_offset 72, -64
	.cfi_offset 73, -56
	.cfi_offset 74, -48
	.cfi_offset 75, -40
	.cfi_offset 76, -32
	.cfi_offset 77, -24
	.cfi_offset 78, -16
	.cfi_offset 79, -8
	nop
	ldp	x21, x22, [sp, 16]
	ldp	x23, x24, [sp, 32]
	ldp	x25, x26, [sp, 48]
	ldp	x27, x28, [sp, 64]
	ldp	d8, d9, [sp, 80]
	ldp	d10, d11, [sp, 96]
	ldp	d12, d13, [sp, 112]
	ldp	d14, d15, [sp, 128]
	ldp	x19, x20, [sp], 144
	.cfi_restore 20
	.cfi_restore 19
	.cfi_restore 78
	.cfi_restore 79
	.cfi_restore 76
	.cfi_restore 77
	.cfi_restore 74
	.cfi_restore 75
	.cfi_restore 72
	.cfi_restore 73
	.cfi_restore 27
	.cfi_restore 28
	.cfi_restore 25
	.cfi_restore 26
	.cfi_restore 23
	.cfi_restore 24
	.cfi_restore 21
	.cfi_restore 22
	.cfi_def_cfa_offset 0
	ret
	.cfi_endproc
