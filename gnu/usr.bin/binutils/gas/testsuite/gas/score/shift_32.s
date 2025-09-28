/*
 * tests for shift instruction relaxation
 *
 * Author: libin
 */

.include "relaxation_macro.h"

.macro _shift_op_pattern insn insn1
  insn_32 "\insn r0,  r0,  0"
  insn_32 "\insn r0,  r0,  31"
  insn_32 "\insn r15, r15, 0"
  insn_32 "\insn r15, r15, 31"

  tran_16_32 "\insn! r0, 0", "\insn r0, r0, 0"

  /* shouldn't alter */
  insn_32 "\insn1 r0,  r0, 0"
  insn_32 "\insn  r0,  r2, 0"
  insn_32 "\insn  r16,  r16, 0"
.endm

.text
/* slli/srli rD,rA,Imm5 -> slli!/srli! rD,Imm5 */
_shift_op_pattern "slli", "slli.c"
_shift_op_pattern "srli", "srli.c"
