# Generates tests to see if the following conditions make the instruction
# undefined:
#
# 1) size == 0
# 2) size == 3 && Q == 0
#
# These patterns can't be created by the assembler so instead manually encode
# them from a starting pattern.
.macro gen_insns_same opc
	.inst \opc
	.inst (\opc & 0xff3fffff) // size == 0
	.inst ((\opc | 0xc00000) & 0xbfffffff) // size == 3 && Q == 0
.endm

# Generates tests to see if the following conditions make the instruction
# undefined:
#
# 1) size == 0 || size == 3
# 2) size == 1 && H == 1 && Q == 0
# 3) size == 2 && (L == 1 || Q == 0)
#
# These patterns can't be created by the assembler so instead manually encode
# them from a starting pattern.
.macro gen_insns_elem opc
	.inst \opc
	.inst (\opc & 0xff3fffff) // size == 0
	.inst (\opc | 0xc00000) // size == 3
	.inst ((\opc | 0x400800) & 0xbf7fffff) // size == 1 && H == 1 && Q == 0
	.inst ((\opc | 0xa00000) & 0xffbfffff) // size == 2 && L == 1
	.inst ((\opc | 0x800000) & 0xbfbfffff) // size == 2 && Q == 0
.endm

# fcmla v1.2d, v2.2d, v3.2d, #0
gen_insns_same 0x6ec3c441

# fcmla v1.2s, v2.2s, v3.2s, #0
gen_insns_same 0x2e83c441

# fcmla v1.4s, v2.4s, v3.4s, #0
gen_insns_same 0x6e83c441

# fcmla v1.4h, v2.4h, v3.4h, #0
gen_insns_same 0x2e43c441

# fcmla v1.8h, v2.8h, v3.8h, #0
gen_insns_same 0x6e43c441

# fcmla v1.4s, v2.4s, v3.s[0], #0
gen_insns_elem 0x6f831041

# fcmla v1.4h, v2.4h, v3.h[0], #0
gen_insns_elem 0x2f431041

# fcmla v1.8h, v2.8h, v3.h[0], #0
gen_insns_elem 0x6f431041

# fcadd v1.2d, v2.2d, v3.2d, #90
gen_insns_same 0x6ec3e441

# fcadd v1.2s, v2.2s, v3.2s, #90
gen_insns_same 0x2e83e441

# fcadd v1.4s, v2.4s, v3.4s, #90
gen_insns_same 0x6e83e441

# fcadd v1.4h, v2.4h, v3.4h, #90
gen_insns_same 0x2e43e441

# fcadd v1.8h, v2.8h, v3.8h, #90
gen_insns_same 0x6e43e441
