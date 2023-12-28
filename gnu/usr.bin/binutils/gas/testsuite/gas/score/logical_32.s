/*
 * tests for logical instruction relaxation
 *
 * Author: libin
 */

.include "relaxation_macro.h"

.macro _logical_op_pattern insn insn1
  insn_32 "\insn r0, r0, r15"

  tran_16_32 "\insn! r0, r15", "\insn r0, r0, r15"

  /* shouldn't alter */
  .set r1
  insn_32 "\insn1  r0,  r0,  r15"
  insn_32 "\insn   r0,  r0,  r16"
  insn_32 "\insn   r16, r16, r0"
  insn_32 "\insn   r16, r16, r17"
  insn_32 "\insn   r0,  r1,  r2"
.endm

.text
/* and/or rD,rA,rB -> and!/or! rD,rA */
_logical_op_pattern "and", "and.c"
_logical_op_pattern "or",  "or.c"
