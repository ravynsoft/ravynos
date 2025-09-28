.syntax unified
// Check argument encoding by having different arguments.
// We use 20 and 11 since their binary encoding is 10100 and 01011
// respectively which ensures that we distinguish between the D/M/N bit
// encoding the first or last bit of the argument.
// q registers are encoded as double their actual number.
vdot.bf16 d0, d20, d11
vdot d11.bf16, d0.bf16, d20.bf16

.macro conversion_type_specifier_check insn, dest, source
\insn\().bf16.f32 \dest, \source
\insn \dest\().bf16, \source\().f32
\insn \dest\().bf16, \source\().f32
.endm
conversion_type_specifier_check vcvtt,s0,s0
conversion_type_specifier_check vcvtb,s0,s0
conversion_type_specifier_check vcvt,d0,q0


// Here we follow the same encoding sequence as above.
// Since the 'M' bit encodes the index and the last register is encoded in 4
// bits that argument has a different number.
vdot.bf16 d11, d0, d4[1]
vdot d0.bf16, d20.bf16, d11.bf16[0]

// vmmla only works on q registers.
// These registers are encoded as double the number given in the mnemonic.
// Hence we choose different numbers to ensure a similar bit pattern as above.
// 10 & 5 produce the bit patterns 10100 & 01010
vmmla.bf16 q10, q5, q0
vmmla q5.bf16, q0.bf16, q10.bf16

vfmat.bf16 q10, q11, q0
vfmat.bf16 q10, q11, d0[3]
vfmat.bf16 q10, q11, d0[0]

vfmab.bf16 q10, q11, q0
vfmab.bf16 q10, q11, d0[3]
vfmab.bf16 q10, q11, d0[0]

// vcvt
// - no condition allowed in arm
// - no condition allowed in thumb outside IT block
// - Condition *allowed* in thumb in IT block
// - different encoding between thumb and arm
vcvt.bf16.f32 d20, q5
vcvt.bf16.f32 d11, q10

// Only works for thumb mode.
.ifdef COMPILING_FOR_THUMB
it ne
vcvtne.bf16.f32 d0, q0
.endif
