# This tests PR ld/17277, wherein ld -shared for cross-section PC-relative
# relocs (other than plain R_ARM_REL32, as in data) produce bogus dynamic
# relocs and TEXTREL markers.

	.syntax unified
	.arm
	.arch armv7-a

	.text
	.globl foo
	.type foo,%function
foo:	movw r0, #:lower16:symbol - 1f - 8
	movt r0, #:upper16:symbol - 1f - 8
1:	add r0, pc
	@ And now a case with a local symbol.
	movw r0, #:lower16:3f - 2f - 8
	movt r0, #:upper16:3f - 2f - 8
2:	add r0, pc
	bx lr

.data
	.globl symbol
	.hidden symbol
symbol:	.long 23
3:	.long 17
