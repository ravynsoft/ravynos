.syntax unified

.arm
	@wback && (n == t)
	ldr r1, [r1, #5]!

	@rt == r15 && rn == r15
	@  && bits<0..1> (immediate) != 00
	ldr r15, [r15, #5]

	@rt == r15 && bits<0..1> (immediate) != 00
	ldr r15, .-0xab7

	@wback && (n == t || n == 15)
	ldr r1, [r1, r2]!
	ldr r2, [r15, r2]!

	@rm == 15
	ldr r1, [r1, r15]

	@wback && (n == t || n == 15)
	str r1, [r1, #10]!
	str r1, [r15, #10]!

	@wback && (n == t || n == 15)
	str r1, [r1, r2]!
	str r1, [r15, r2]!

