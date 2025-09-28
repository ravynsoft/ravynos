# Define a function with all "compressed" (dc, bc and ic) references.

	.abicalls
	.option	pic0

	.include "compressed-plt-1.s"

	.macro	test_one, name, types
	.if	(\types) & DC
	jal	\name
	nop
	.if	micromips
	.ifdef	o32
	j	\name
	nop
	.endif
	.endif
	.endif
	.if	(\types) & BC
	.if	micromips
	bal	\name
	nop
	.endif
	.ifdef	o32
	b	\name
	nop
	.endif
	.endif
	.if	(\types) & IC
	lw	$2, %call16(\name)($3)
	.endif
	.endm

	.if	micromips
	.set	micromips
	.else
	.set	mips16
	.endif

	.section .text.a, "ax", @progbits
	.globl	testc
	.ent	testc
	.set	noreorder
testc:
	test_all
	jr	$31
	.end	testc
