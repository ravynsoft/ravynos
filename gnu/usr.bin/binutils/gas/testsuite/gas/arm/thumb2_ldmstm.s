.syntax unified
.thumb
ldmstm:
	ldmia sp!, {r0}
	ldmia sp!, {r8}
	ldmia r1, {r9}
	ldmia r2!, {ip}
	ldmdb sp!, {r2}
	ldmdb sp!, {r8}
	ldmdb r6, {r4}
	ldmdb r6, {r8}
	ldmdb r2!, {r4}
	ldmdb r2!, {ip}
	stmia sp!, {r3}
	stmia sp!, {r9}
	stmia r3, {ip}
	stmia r4!, {ip}
	stmdb sp!, {r3}
	stmdb sp!, {r9}
	stmdb r7, {r5}
	stmdb r6, {ip}
	stmdb r6!, {fp}
	stmdb r5!, {r8}

	@ Valid Thumb-2 encodings of LDM/LDMIA/LDMFD as specified by section
	@ A8.6.53 of the ARM ARM
	ldmia r0!, {r1-r3}	@ Encoding T1
	ldmia r0, {r0-r3}	@ Encoding T1
	ldmia r0!, {r1}		@ Encoding T1
	ldmia r0, {r8-r11}	@ Encoding T2
	ldmia.w r0!, {r1-r3}	@ Encoding T2
	ldmia r0!, {r8-r11}	@ Encoding T2
	ldmia r0!, {r12, r14}	@ Encoding T2
	ldmia r0!, {r12, pc}	@ Encoding T2
	it eq
	ldmiaeq r0!, {r12, pc}	@ Encoding T2

	@ Valid Thumb-2 encodings of STM/STMIA/STMEA as specified by section
	@ A8.6.189 of the ARMARM.
	stmia r0!, {r0-r3}	@ Encoding T1, Allowed as r0 is lowest reg
	stmia r0!, {r4-r7}	@ Encoding T1
	stmia.w r0!, {r4-r7}	@ Encoding T2
	stmia r0!, {r8-r11}	@ Encoding T2
	stmia r0, {r0-r3}	@ Encoding T2
	stmia r0, {r8-r11}	@ Encoding T2

	@ The following are technically UNPREDICTABLE if we assemble them
	@ as written, but gas translates (stm|ldm) rn(!), {rd} into an
	@ equivalent, and well-defined, (ldr, str) rd, [rn], (#4).
	ldmia.w r0!, {r1}	@ ldr.w r1, [r0], #4
	ldmia.w r0, {r1}	@ ldr.w r1, [r0]
	ldmia r8!, {r9}		@ ldr.w r9, [r8], #4
        ldmia r8, {r9}		@ ldr.w r9, [r8]
	stmia.w r0!, {r1}	@ str.w r1, [r0], #4
	stmia r0, {r1}		@ T1 str r1, [r0]
	ldmia r1, {r2}		@ T1 ldr r2, [r1]
	ldmia r0, {r7}		@ T1 ldr r7, [r0]
	stmia sp, {r7}		@ T1 str r7, [sp]
	stmia sp, {r0}		@ T1 str r0, [sp]
	ldmia sp, {r7}		@ T1 ldr r7, [sp]
	ldmia sp, {r0}		@ T1 ldr r0, [sp]
	stmia r8!, {r9}		@ str.w r9, [r8], #4
	stmia r8, {r9}		@ str.w r9, [r8]
