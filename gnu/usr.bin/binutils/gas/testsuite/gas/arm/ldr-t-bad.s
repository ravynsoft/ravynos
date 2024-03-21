.syntax unified
.arch armv7-a
.thumb

	@ldr-immediate

	@wback && (n == t)
	ldr r1, [r1, #5]!

	@rt == r15 && rn == r15
	@  && bits<0..1> (immediate) != 00
	ldr r15, [r15, #5]

	@inITBlock && rt == 15 && !lastInITBlock
	ittt ge
	ldrge r15, [r15, #4]
	nopge
	nopge

	@ldr-literal


	@inITBlock && rt == 15 && !lastInITBlock
	ittt ge
	ldrge r15, .0x4
	nopge
	nopge

	@rt == r15 && bits<0..1> (immediate) != 00
	ldr r15, .-0xab7

	@ldr-register

	@inITBlock && rt == 15 && !lastInITBlock
	ittt ge
	ldrge r15, [r15, r1]
	nopge
	nopge

	@rm == 13 || rm == 15
	ldr r1, [r2, r13]
	ldr r2, [r2, r15]

	@str-immediate

	@rt == 15 || rn == 15
	str r15, [r1, #10]
	str r1, [r15, #10]

	@wback && (n == t)
	str r1, [r1, #10]!

	@str-register

	@rt == 15 || rm == 13 || rm == 15
	str r15, [r1, r2]
	str r1, [r2, r13]
	str r1, [r2, r15]

	@ PR 14260
	ldrt r0, =0x0
