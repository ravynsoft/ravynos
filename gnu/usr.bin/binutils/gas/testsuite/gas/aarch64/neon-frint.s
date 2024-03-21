// Test file for AArch64 GAS -- Advanced SIMD floating-point round to integral
// instructions.

	.macro frint_main rd rn
	.irp rounding_mode, N, A, P, M, X, Z, I
	.irp reg_shape, 2S, 4S, 2D
	frint\rounding_mode	\rd\().\reg_shape, \rn\().\reg_shape
	.endr
	.endr
	.endm

	.text
	frint_main	v7, v7
