	.thumb
	.syntax unified
	.thumb_func
thumb2_ldr:
	# These must be encoded into mov.w despite constant and register being
	# small enough as ldr should not generate a flag-setting instruction.
	ldr r0,=0x00
	ldr r1,=0x08
	ldr r2,=0x51
	ldr r3,=0x1F
	ldr r4,=0x2F
	ldr r5,=0x3F
	ldr r6,=0x80
	ldr r7,=0xFF
	# These shall be encoded into mov.w since register cannot be encoded in
	# 3 bits
	ldr r8,=0x00
	ldr r9,=0x08
	ldr r10,=0x51
	ldr r11,=0x1F
	ldr r12,=0x2F
	ldr r14,=0x80
	# These shall be encoded into movw since immediate cannot be encoded
	# with mov.w
	ldr r8,=0xFFFF
	ldr r9,=0xF0F0
	# These should be encoded as ldr since mov immediate is unpredictable
	# for sp and pc
	ldr r13,=0x3F
	ldr r15,=0xFF
