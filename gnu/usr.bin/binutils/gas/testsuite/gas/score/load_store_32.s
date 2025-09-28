/*
 * tests for load/store instruction relaxation
 *
 * Author: libin
 */

.include "relaxation_macro.h"

.macro _ls_op_pattern insn
.balign 2
  insn_32 "\insn r0,  [r0,0]"
  insn_32 "\insn r15, [r0,0]"
  insn_32 "\insn r0,  [r7,0]"
  insn_32 "\insn r15, [r7,0]"
/* NOTE: offset MUST be word aligned */
  insn_32 "\insn r0,  [r0,124]"
  insn_32 "\insn r15, [r0,124]"
  insn_32 "\insn r0,  [r7,124]"
  insn_32 "\insn r15, [r7,124]"

  tran_16_32 "\insn! r0,[r0,124]", "\insn r0,[r0,124]"

  /* shouldn't alter */
  insn_32 "\insn r16, [r0, 0]"
  insn_32 "\insn r0,  [r8, 124]"
  insn_32 "\insn r16, [r8, 124]"
  insn_32 "\insn r0,  [r7, -1]"
  insn_32 "\insn r0,  [r7, 128]"
.endm

.text
/* lw/sw rD,[rA,SImm15] -> lw!/sw! rD,[rA,Imm5] */
_ls_op_pattern "lw"
_ls_op_pattern "sw"

/* ldi rD,SImm16 -> ldiu! rD,Imm6 */
.balign 2
insn_32 "ldi r0,  0"
insn_32 "ldi r15, 0"
insn_32 "ldi r0,  31"
insn_32 "ldi r15, 31"

tran_16_32 "ldiu! r0, 0", "ldi r0, 0"

/* shouldn't alter */
insn_32 "ldi r16, 0"
insn_32 "ldi r0,  -1"
insn_32 "ldi r0,  32"
insn_32 "ldi r16, 32"

/*
 * lw rD,[rA]+,SImm12 -> pop! rD
 *
 * r0: stack pointer(sp)
 */
insn_32 "lw r2,   [r0]+, 4"
insn_32 "lw r15,  [r0]+, 4"

/* shouldn't alter */
insn_32 "lw r16, [r0]+, 4"
insn_32 "lw r4,  [r2]+, 4"
insn_32 "lw r4,  [r0]+, -4"

/* sw rD,[rA,SImm12]+ -> push! rD */
insn_32 "sw r2,  [r0, -4]+"
insn_32 "sw r15, [r0, -4]+"

/* shouldn't alter */
insn_32 "sw r16, [r0, -4]+"
insn_32 "sw r4,  [r2, -4]+"
insn_32 "sw r4,  [r0, 4]+"
