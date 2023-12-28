@	Test file for ARM load/store instructions with pc as the base register

	.text
	.syntax unified
	.align 2
	ldr r1, [pc, #-8]
	ldr r1, [pc, r2]
	ldrb r1, [pc, r2]
	ldrd r0, r1, [pc, r2]
	ldrh r1, [pc, r2]
	ldrsb r1, [pc, r2]
	ldrsh r1, [pc, r2]

	pld [pc, #-8]
	pld [pc, r1]

	pli [pc, #-8]
	pli [pc, r1]

	str r1, [pc, #4]
	str r1, [pc, r2]
	strb r1, [pc, r2]
	strd r0, r1, [pc, r2]
	strh r1, [pc, r2]
