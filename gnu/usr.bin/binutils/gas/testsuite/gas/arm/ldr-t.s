.syntax unified
.arch armv7-a
.thumb
	.global foo
foo:
	.align 4
	@ldr-immediate

	@!wback && (n == t)
	ldr r1, [r1, #5]

	@wback && !(n == t)
	ldr r1, [r2, #5]!

	@!(rt == r15) && rn == r15
	@  && bits<0..1> (immediate) != 00
	ldr r1, [r15, #5]

	@rt == r15 && !(rn == r15)
	@  && bits<0..1> (immediate) != 00
	ldr r15, [r1, #5]

	@rt == r15 && rn == r15
	@  && bits<0..1> (immediate) == 00
	ldr r15, [r15, #4]

	@inITBlock && !(rt == 15) && !lastInITBlock
	ittt ge
	ldrge r1, [r15, #4]
	nopge
	nopge

	@inITBlock && rt == 15 && lastInITBlock
	it ge
	ldrge r15, [r15, #4]

	@ldr-literal

	@inITBlock && !(rt == 15) && !lastInITBlock
	ittt ge
	ldrge r1, .-0xab4
	nopge
	nopge

	@inITBlock && (rt == 15) && lastInITBlock
	it ge
	ldrge r15, .-0xab4

	@!(rt == r15) && bits<0..1> (immediate) != 00
	ldr r1, .-0xab7

	@rt == r15 && bits<0..1> (immediate) == 00
	ldr r15, .-0xab4

	@ldr-register

	@inITBlock && !(rt == 15) && !lastInITBlock
	ittt ge
	ldrge r1, [r2, r1]
	nopge
	nopge

	@inITBlock && (rt == 15) && lastInITBlock
	it ge
	ldrge r15, [r2, r1]

	@!(rm == 13 || rm == 15)
	ldr r1, [r2, r3]

	@str-immediate

	@!(rt == 15 || rn == 15)
	str r1, [r2, #10]

	@!wback && (n == t)
	str r1, [r1, #10]

	@wback && !(n == t)
	str r1, [r2, #10]!

	@str-register

	@!(rt == 15 || rm == 13 || rm == 15)
	str r1, [r2, r3]

