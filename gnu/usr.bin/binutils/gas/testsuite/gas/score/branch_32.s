/*
 * tests for branch instruction relaxation
 *
 * Author: libin
 */

.include "relaxation_macro.h"

.macro _b_op_pattern insn insn1
.balign 2

/*
 * for local label 1, assembler should NOT alter instructions before .skip;
 * but it SHOULD alter instructions afte it.
 */
1:
  insn_16    "\insn! 1b"
  tran_16_32 "\insn! 1b", "\insn 1b"
  insn_16    "\insn1 1b"
.skip 512
  insn_16    "\insn! 1b"
  tran_16_32 "\insn! 1b", "\insn 1b"
  insn_16    "\insn1 1b"

/* 
 * for local label 2, assembler SHOULD alter instructions before .skip;
 * but it should NOT alter instructions after it.
 */
  insn_16    "\insn! 2f"
  tran_16_32 "\insn! 2f", "\insn 2f"
  insn_16    "\insn1 2f"
.skip 511
  insn_16    "\insn! 2f"
  tran_16_32 "\insn! 2f", "\insn 2f"
  insn_16    "\insn1 2f"
2:
  nop!

/* tests for boundary */
3:
.skip 512
  insn_16 "\insn! 3b"
  insn_16 "\insn! 3b"

  insn_16 "\insn! 4f"
  insn_16 "\insn! 4f"
.skip 511
4:
  nop!
.endm

.macro _br_op_pattern insn
.balign 2
  insn_32 "\insn r0"
  insn_32 "\insn r15"

  tran_16_32 "\insn! r0", "\insn r0"

  /* shouldn't alter */
  insn_32 "\insn r16"
  insn_32 "\insn r31"
.endm

.macro _bcmp_op_pattern1 insn
.balign 2

/* as will give "Using temp register(r1)" warning if you using r1 */

/*
 * for local label 1, assembler should NOT alter instructions before .skip;
 * but it SHOULD alter instructions afte it.
 */
1:
  insn_32 "\insn r0,  r15, 1b"
  insn_32 "\insn r15, r16, 1b"
  insn_32 "\insn r15, r31, 1b"
  insn_32 "\insn r16, r31, 1b"
.skip 512
  insn_32 "\insn r0,  r15, 1b"
  insn_32 "\insn r15, r16, 1b"
  insn_32 "\insn r15, r31, 1b"
  insn_32 "\insn r16, r31, 1b"

/* 
 * for local label 2, assembler SHOULD alter instructions before .skip;
 * but it should NOT alter instructions after it.
 */
  insn_32 "\insn r0,  r15, 2f"
  insn_32 "\insn r15, r16, 2f"
  insn_32 "\insn r15, r31, 2f"
  insn_32 "\insn r16, r31, 2f"
.skip 511
  insn_32 "\insn r0,  r15, 2f"
  insn_32 "\insn r15, r16, 2f"
  insn_32 "\insn r15, r31, 2f"
  insn_32 "\insn r16, r31, 2f"
2:
  nop!

/* tests for boundary */
3:
.skip 512
  insn_32 "\insn r0,  r15, 3b"
  insn_32 "\insn r16, r15, 3b"

  insn_32 "\insn r0,  r15, 4f"
  insn_32 "\insn r16, r15, 4f"
.skip 511
4:
.endm 

.macro _bcmp_op_pattern2 insn
.balign 2

/* as will give "Using temp register(r1)" warning if you using r1 */

/*
 * for local label 1, assembler should NOT alter instructions before .skip;
 * but it SHOULD alter instructions afte it.
 */
1:
  insn_32 "\insn r0,  1b"
  insn_32 "\insn r15, 1b"
  insn_32 "\insn r16, 1b"
  insn_32 "\insn r31, 1b"
.skip 512
  insn_32 "\insn r0,  1b"
  insn_32 "\insn r15, 1b"
  insn_32 "\insn r16, 1b"
  insn_32 "\insn r31, 1b"

/* 
 * for local label 2, assembler SHOULD alter instructions before .skip;
 * but it should NOT alter instructions after it.
 */
  insn_32 "\insn r0,  2f"
  insn_32 "\insn r15, 2f"
  insn_32 "\insn r16, 2f"
  insn_32 "\insn r31, 2f"
.skip 511
  insn_32 "\insn r0,  2f"
  insn_32 "\insn r15, 2f"
  insn_32 "\insn r16, 2f"
  insn_32 "\insn r31, 2f"
2:
  nop!

/* tests for boundary */
3:
.skip 512
  insn_32 "\insn r0,  3b"
  insn_32 "\insn r16, 3b"

  insn_32 "\insn r0,  4f"
  insn_32 "\insn r16, 4f"
.skip 511
4:
.endm 

.text
/* b Disp19 <-> b! Disp9 */
_b_op_pattern "bgtu", "bgtul"
_b_op_pattern "bleu", "bleul"
_b_op_pattern "beq",  "beql"
_b_op_pattern "bne",  "bnel"
_b_op_pattern "bgt",  "bgtl"
_b_op_pattern "ble",  "blel"
_b_op_pattern "bcnz", "bcnzl"
_b_op_pattern "b",    "bl"

/* br rD <-> br! rD */
_br_op_pattern "br"
_br_op_pattern "brl"

/* bcmpeq/bcmpne rA,rB,Disp9 -> cmp/cmp! rA, rB; beq/bne Disp19 */
_bcmp_op_pattern1 "bcmpeq"
_bcmp_op_pattern1 "bcmpne"

/* bcmpeqz/bcmpnez rA,Disp9 -> cmpi! rA, 0; beq/bne Disp19 */
_bcmp_op_pattern2 "bcmpeqz"
_bcmp_op_pattern2 "bcmpnez"
