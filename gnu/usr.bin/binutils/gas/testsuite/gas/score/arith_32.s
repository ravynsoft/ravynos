/*
 * tests for arithmetic instruction relaxation 
 *
 * Author: libin
 */

.include "relaxation_macro.h"

.macro _arith_op_pattern insn insn1
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
/* add rD,rA,rB -> add! rD,rA */
_arith_op_pattern "add", "add.c"

/* sub rD,rA,rB -> sub rD,rA */
_arith_op_pattern "sub", "sub.c"

/* addi rD,SImm16 -> addi! rD,SImm6 */
insn_32 "addi r0,  -32"
insn_32 "addi r0,  31"
insn_32 "addi r15, -32"
insn_32 "addi r15, 31"

tran_16_32 "addi! r0,-32", "addi r0,-32"

/* shouldn't alter */
insn_32 "addi.c r0,  -32"
insn_32 "addi   r0,  -33"
insn_32 "addi   r0,  32"
insn_32 "addi   r16, -32"
insn_32 "addi   r16, 31"
