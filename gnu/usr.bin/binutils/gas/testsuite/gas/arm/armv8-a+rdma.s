	.syntax unified
	.text

	.macro vect_inst I T R
	\I\().\T \R\()0, \R\()1, \R\()2
	.endm

	.macro scalar_inst I T R N
	\I\().\T \R\()0, \R\()1, d\()2[\N\()]
	.endm

	.text
	.arm
A1:
	.irp inst, vqrdmlah, vqrdmlsh
	.irp type, s16, s32
	.irp reg, d, q
	vect_inst \inst \type \reg
	.endr
	.endr
	.endr

	.irp inst, vqrdmlah, vqrdmlsh
	.irp reg, d, q
	.irp idx, 0, 1, 2, 3
	scalar_inst \inst s16 \reg \idx
	.endr
	.endr
	.irp reg, d, q
	.irp idx, 0, 1
	scalar_inst \inst s32 \reg \idx
	.endr
	.endr
	.endr

	.text
	.thumb
T1:
	.irp inst, vqrdmlah, vqrdmlsh
	.irp type, s16, s32
	.irp reg, d, q
	vect_inst \inst \type \reg
	.endr
	.endr
	.endr

	.irp inst, vqrdmlah, vqrdmlsh
	.irp reg, d, q
	.irp idx, 0, 1, 2, 3
	scalar_inst \inst s16 \reg \idx
	.endr
	.endr
	.irp reg, d, q
	.irp idx, 0, 1
	scalar_inst \inst s32 \reg \idx
	.endr
	.endr
	.endr
