
	.text
	.globl main
	.type main,#function
main:
	stp	x29, x30, [sp,#-16]!
	mov	x29, sp
	movz	x0, #:abs_g3:bar
	movk	x0, #:abs_g2_nc:bar
	movk	x0, #:abs_g1_nc:bar
	movk	x0, #:abs_g0_nc:bar
	add     x0, x0, #0x0
	bl      foo
	ldp     x29, x30, [sp],#16
	ret
