.syntax unified
.thumb
ldmstm_bad:
	@ UNPREDICTABLE Thumb-2 encodings of LDM/LDMIA/LDMFD as specified
	@ by section A8.6.53 of the ARMARM.
	ldmia r15, {r0-r3}	@ Encoding T2, UNPREDICTABLE
	ldmia r15!, {r0-r3}	@ Encoding T2, UNPREDICTABLE
	ldmia r1, {r14, r15}	@ Encoding T2, UNPREDICTABLE
	ldmia r0!, {r0-r3}	@ Encoding T2, UNPREDICTABLE

	itt eq
	ldmiaeq r0, {r12, r15}	@ Encoding T2, UNPREDICTABLE
	ldmiaeq r0!, {r0, r1}	@ Encoding T2, UNPREDICTABLE

	@ UNPREDICTABLE Thumb-2 encodings of STM/STMIA/STMEA as specified
	@ by section A8.6.189 of the ARMARM.
	stmia.w r0!, {r0-r3}	@ Encoding T2, UNPREDICTABLE
	stmia r1!, {r0-r3}	@ Encoding T1, r1 is UNKNOWN
	stmia r15!, {r0-r3}	@ Encoding T2, UNPREDICTABLE
	stmia r15, {r0-r3}	@ Encoding T2, UNPREDICTABLE
	stmia r8!, {r0-r11}     @ Encoding T2, UNPREDICTABLE

	@ The following are technically UNDEFINED, but gas converts them to
	@ an equivalent, and well-defined instruction automatically.
	@stmia.w r0!, {r1}	@ str.w r1, [r0], #4
	@stmia r8!, {r9}	@ str.w r9, [r8], #4
	@stmia r8, {r9}		@ str.w r9, [r8]
	@ldmia.w r0!, {r1}	@ ldr.w r1, [r0], #4
	@ldmia r8!, {r9}	@ ldr.w r9, [r8], #4
	@ldmia r8, {r9}		@ ldr.w r9, [r8]
