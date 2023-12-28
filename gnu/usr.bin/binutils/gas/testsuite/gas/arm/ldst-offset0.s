@	Test file for ARM load/store instructions with 0 offset

	.text
	.syntax unified
	ldr r1, [r2, #-0]
	ldr r1, [r2, #-1+1]

	ldr r1, [r2, #1-1]
	ldr r1, [r2, #0]

	ldr r1, [r2, #-0]!
	ldr r1, [r2, #-1+1]!

	ldr r1, [r2, #1-1]!
	ldr r1, [r2, #0]!

	ldr r1, [r2], #-0
	ldr r1, [r2], #-1+1

	ldr r1, [r2], #1-1
	ldr r1, [r2], #0

	ldr r1, [r2]!
	ldr r1, [r2]

	ldrbt r1, [r2], #0
	ldrbt r1, [r2], #-0

	ldrbt r1, [r2]

	ldclpl p3, c5, [r6, #-0]
	ldclpl p3, c5, [r6, #0]

	str r1, [r2, #-0]
	str r1, [r2, #-1+1]

	str r1, [r2, #1-1]
	str r1, [r2, #0]

	str r1, [r2, #-0]!
	str r1, [r2, #-1+1]!

	str r1, [r2, #1-1]!
	str r1, [r2, #0]!

	str r1, [r2], #-0
	str r1, [r2], #-1+1

	str r1, [r2], #1-1
	str r1, [r2], #0

	str r1, [r2]!
	str r1, [r2]

	strbt r1, [r2], #0
	strbt r1, [r2], #-0

	strbt r1, [r2]

	stclpl p3, c5, [r6, #-0]
	stclpl p3, c5, [r6, #0]

	ldr	r0,1f
	ldr	r0,1f
	ldr	r0,1f
1:	.word	0
