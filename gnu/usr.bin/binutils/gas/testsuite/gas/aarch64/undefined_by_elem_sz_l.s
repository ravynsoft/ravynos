# Generates tests to see if setting bit 22 (sz) and 21 (L) together correctly
# marks the instruction as undefined.  This pattern can't be created by the
# assembler so instead manually encode it.
.macro gen_insns opc
	.inst \opc
	.inst (\opc | 0x600000)
.endm

# fmul  s0, s0, v16.s[0]
gen_insns 0x5f909000

# fmla  s0, s0, v16.s[0]
gen_insns 0x5f901000

# fmls  s0, s0, v16.s[0]
gen_insns 0x5f905000

# fmulx s0, s0, v16.s[0]
gen_insns 0x7f909000

# fmul  d0, d0, v16.d[0]
gen_insns 0x5fd09000

# fmla  d0, d0, v16.d[0]
gen_insns 0x5fd01000

# fmls  d0, d0, v16.d[0]
gen_insns 0x5fd05000

# fmulx d0, d0, v16.d[0]
gen_insns 0x7fd09000

# fmul  v0.4s, v0.4s, v16.s[0]
gen_insns 0x4f909000

# fmla  v0.4s, v0.4s, v16.s[0]
gen_insns 0x4f901000

# fmls  v0.4s, v0.4s, v16.s[0]
gen_insns 0x4f905000

# fmulx v0.4s, v0.4s, v16.s[0]
gen_insns 0x6f909000

# fmul  v0.2d, v0.2d, v16.d[0]
gen_insns 0x4fd09000

# fmla  v0.2d, v0.2d, v16.d[0]
gen_insns 0x4fd01000

# fmls  v0.2d, v0.2d, v16.d[0]
gen_insns 0x4fd05000

# fmulx v0.2d, v0.2d, v16.d[0]
gen_insns 0x6fd09000
