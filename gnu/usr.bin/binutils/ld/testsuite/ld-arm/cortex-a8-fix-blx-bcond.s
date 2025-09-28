	.syntax unified
	.cpu cortex-a8
	.thumb
	.text

	@ expansion 32 bytes
	.macro bw1
1:
	add.w r0, r1, r2
	blx.w arm_target
	add.w r0, r1, r2
	blx.w arm_target
	add.w r0, r1, r2
	blx.w arm_target
	add.w r0, r1, r2
	blx.w arm_target
	.endm

	@ expansion 128 bytes
	.macro bw2
	bw1
	bw1
	bw1
	bw1
	.endm

	@ expansion 32 bytes
	.macro bw3
1:
	add.w r0, r1, r2
	bne.w 1b
	add.w r0, r1, r2
	bne.w 1b
	add.w r0, r1, r2
	bne.w 1b
	add.w r0, r1, r2
	bne.w 1b
	.endm

	@ expansion 128 bytes
	.macro bw4
	bw3
	bw3
	bw3
	bw3
	.endm

	.align  3
	.global _start

	.thumb
	.thumb_func
	.type   _start, %function
_start:
	nop

	@ Trigger Cortex-A8 erratum workaround with b<cond> instructions.
	bw4
	bw4

	nop

	.rept 957
	nop.w
	.endr

	.arm
arm_target:
	add r3, r4, r5
	bx lr

	.thumb
bl_insns:

	nop

	@ ...and again with bl instructions.
	bw2
	bw2

	bx      lr
