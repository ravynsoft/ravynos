# Define a function with all "uncompressed" (du, bu and iu) references.

	.abicalls
	.option	pic0

	.include "compressed-plt-1.s"

	.macro	test_one, name, types
	.if	(\types) & DU
	jal	\name
	nop
	j	\name
	nop
	.endif
	.if	(\types) & BU
	bal	\name
	nop
	b	\name
	nop
	.endif
	.if	(\types) & IU
	lw	$2, %call16(\name)($3)
	.endif
	.endm

	.section .text.b, "ax", @progbits
	.globl	testu
	.ent	testu
	.set	noreorder
testu:
	test_all
	jr	$31
	.end	testu
