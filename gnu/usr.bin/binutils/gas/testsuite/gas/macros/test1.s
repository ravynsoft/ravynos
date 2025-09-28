	.macro	m arg1 arg2
	.globl	\arg1
	\arg1 == \arg2
	.endm

	m s_not_a_reg_1,1
	m s_not_a_reg_2,2
