	.arch armv7-a
	.global _start
	.syntax unified
	.text
	.thumb

	.macro do_calls
	@ The following four instructions are accepted by gas, but generate
	@ meaningless code.
	@bl.w arm0
	@bl.w arm4
	@nop
	@bl.w arm0
	@bl.w arm4
	@nop
	blx.w arm0
	blx.w arm4
	nop
	blx.w arm0
	blx.w arm4
	nop
	bl.w thumb0
	bl.w thumb2
	bl.w thumb4
	bl.w thumb6
	nop
	bl.w thumb0
	bl.w thumb2
	bl.w thumb4
	bl.w thumb6
	nop
	@ These eight are all accepted by gas, but generate bad code.
	@blx.w thumb0
	@blx.w thumb2
	@blx.w thumb4
	@blx.w thumb6
	@nop
	@blx.w thumb0
	@blx.w thumb2
	@blx.w thumb4
	@blx.w thumb6
	.endm

	.thumb_func
	.align 3
_start:
	do_calls

	.arm
	.align 3
arm0:
	bx lr

	.align 3
	nop
arm4:
	bx lr

	.thumb
	.thumb_func
	.align 3
thumb0:
	bx lr

	.thumb_func
	.align 3
	nop
thumb2:
	bx lr

	.thumb_func
	.align 3
	nop
	nop
thumb4:
	bx lr

	.thumb_func
	.align 3
	nop
	nop
	nop
thumb6:
	bx lr

backwards:
	do_calls
