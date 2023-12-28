.syntax unified

// Test warnings about type specifier being incorrect.
vdot.b16  d0, d0, d0
vmmla  q0.b16, q0, q0
vdot.bf32 d0, d0, d0[1]
vdot d0.bf32, d0, d0
vdot d0.bf32, d0.bf16, d0.bf16

// Test conditions are not allowed in ARM.
vdotne d0, d0, d0
vdotne d0, d0, d0[1]
vmmlane q0, q0, q0
vfmatne.bf16 q0, d0, d0
vfmatne.bf16 q0, d0, d0[0]
vfmabne.bf16 q0, d0, d0
vfmabne.bf16 q0, d0, d0[0]
vcvtne.bf16.f32 d0, q0

// d register out of range
vdot d32, d0, d0
vdot d0, d32, d0
vdot d0, d0, d32
vdot d32, d0, d0[0]
vdot d0, d32, d0[0]
vdot d0, d0, d16[0]
vcvtne.bf16.f32 d32, q0

// q register out of range
vdot q16, q0, q0
vdot q0, q16, q0
vdot q0, q0, q16
vdot q16, q0, d0[0]
vdot q0, q16, d0[0]
vmmla q16, q0, q0
vmmla q0, q16, q0
vmmla q0, q0, q16
vfmab.bf16 q16, d0, d0
vfmab.bf16 q16, d0, d0[0]
vfmab.bf16 q0, q32, d0
vfmab.bf16 q0, q32, d0[0]
vfmab.bf16 q0, q0, d8[0]
vfmat.bf16 q16, d0, d0
vfmat.bf16 q16, d0, d0[0]
vfmat.bf16 q0, q32, d0
vfmat.bf16 q0, q32, d0[0]
vfmat.bf16 q0, q0, d8[0]
vcvt.bf16.f32 d0, q16

// Incorrect set of arguments
vdot q0, q0, d5
vdot q0, d5, q0
vdot d5, q0, q0
vdot q0, d5, q0[0]
vdot d5, q0, q0[0]
vmmla q0, q0, d5
vmmla q0, d5, q0
vmmla d5, q0, q0
vfmab.bf16 d0, q0, d0
vfmab.bf16 d0, q0, d0[0]
vfmat.bf16 d0, q0, d0
vfmat.bf16 d0, q0, d0[0]
vcvt.bf16.f32 q0, d0

// vdot index out of range
vdot q0, q0, d0[2]

// vfma<bt> index out of range
vfmab.bf16 q0, d0, d0[4]
vfmat.bf16 q0, d0, d0[4]

// Non neon encodings (this file gets assembled more than once but with
// different flags, providing different error messages each time).

// Type specifier warnings
.macro conversion_type_specifier_check insn, dest, source
\insn\().b16.f32 \dest, \source
\insn\().bf32.f32 \dest, \source
\insn \dest\().b16, \source\().f32
\insn \dest\().bf32, \source\().f32
\insn \dest\().f32, \source\().bf16
.endm

conversion_type_specifier_check vcvtb, s0, s0
conversion_type_specifier_check vcvtt, s0, s0
conversion_type_specifier_check vcvt, d0, q0

// Conditions allowed (and checked in the "Valid" source file).

// Incorrect set of operands & registers out of range
.macro bad_args insn
\insn\().bf16.f32 s0, s0, #0
\insn\().bf16.f32 s0, s0, #1
\insn\().bf16.f32 d0, s0
\insn\().bf16.f32 s0
\insn\().bf16.f32 s0, s0, s0, s0
\insn\().bf16.f32 s0, s0, s0
\insn\().bf16.f32 s0, s32
\insn\().bf16.f32 s32, s32
.endm
bad_args vcvtt
bad_args vcvtb

// Allowed in thumb mode but not allowed in arm mode.
it ne
vcvtne.bf16.f32 d0, q0

// Ensure these instructions are not allowed to have a conditional suffix.
ittt ne
vdotne.bf16 d0, d20, d11
vdotne.bf16 d0, d20, d11[1]
vmmlane.bf16 q0, q0, q0

// Ensure we are warned these instructions are UNPREDICTABLE in an IT block in
// thumb.
ittt ne
vdot.bf16 d0, d20, d11
vdot.bf16 d0, d20, d11[1]
vmmla.bf16 q0, q0, q0
