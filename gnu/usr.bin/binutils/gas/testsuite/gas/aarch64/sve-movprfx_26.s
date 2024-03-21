/* Checks the special cases for FCVT and LSL.
   Has invalid usages.  Diagnosis required.  */
  .text
  .arch armv8-a+sve

f:
  .macro test_cvt, insn

  /* Not OK, 64-bit operation, upper 32-bits cleared.  */
  movprfx Z0.S, P1/M, Z1.S
  \insn Z0.S, P1/M, Z2.D

  /* OK, 64-bit operation, upper 32-bits cleared.  */
  movprfx Z0.D, P1/M, Z1.D
  \insn Z0.S, P1/M, Z2.D

  /* Not OK, 64-bit operation ignoring 32-bits.  */
  movprfx Z0.S, P1/M, Z1.S
  \insn Z0.D, P1/M, Z2.S

  /* OK, 64-bit operation ignoring 32-bits.  */
  movprfx Z0.D, P1/M, Z1.D
  \insn Z0.D, P1/M, Z2.S
  .endm test_cvt

  .macro test_shift, insn
  /* OK, 8-bit operation.  */
  movprfx Z0.B, P1/M, Z1.B
  \insn Z0.B, P1/M, Z0.B, Z2.D

  /* Not Ok, destination register sizes don't match.  */
  movprfx Z0.D, P1/M, Z1.D
  \insn Z0.B, P1/M, Z0.B, Z2.D
  .endm test_shift

  test_cvt fcvt
  test_cvt fcvtzs
  test_cvt fcvtzu
  test_cvt scvtf
  test_cvt ucvtf

  test_shift lsl
  test_shift lsr
  test_shift asr

  ret

