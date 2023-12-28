
        asr     r0, r1, r2, ror #5
        ror     r0, r1, r2, lsl r3

        .thumb_func
foo:
        ror     r0, r0, r2, lsl #1
        lsl     r0, r0, r2, lsl #1
        lsl     r0, r0, r2, asr r0

        .syntax unified
	
        ror     r0, r1, r2, lsl #1
        lsl     r0, r1, r2, lsl #1
        lsl     r0, r1, r2, asr r0
