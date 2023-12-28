	.extInstruction custom0, 0x07, 0x1a, SUFFIX_COND|SUFFIX_FLAG, SYNTAX_3OP
	.extInstruction custom1, 0x07, 0x1b, SUFFIX_FLAG, SYNTAX_2OP

	.extCoreRegister mlx, 57, r|w, can_shortcut
	.extCondCode tst, 0x10
	.extAuxRegister aux_test, 0x41, r|w

	custom0.tst	mlx,mlx,r0
	custom1		mlx,r0

	lr	r0, [aux_test]
