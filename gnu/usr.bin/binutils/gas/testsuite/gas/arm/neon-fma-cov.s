	.arm
	.syntax unified
	.text

	.macro regs3_1 op opq vtype
	\op\vtype q0,q0,q0
	\opq\vtype q0,q0,q0
	\op\vtype d0,d0,d0
	.endm

	regs3_1 vfma vfma .f32
	regs3_1 vfms vfms .f32
