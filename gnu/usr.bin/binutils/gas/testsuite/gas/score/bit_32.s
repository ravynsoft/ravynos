/*
 * tests for bit operations' instruction relaxation
 *
 * Author: libin
 */

.include "relaxation_macro.h"

.macro _bit_op_pattern insn insn1
  insn_32 "\insn r0,  r0,  0"
  insn_32 "\insn r0,  r0,  0x1f"
  insn_32 "\insn r15, r15, 0"
  insn_32 "\insn r15, r15, 0x1f"

  tran_16_32 "\insn! r0,0", "\insn r0,r0,0"

  /* shouldn't alter */
  insn_32 "\insn1 r0,  r0,  0"
  insn_32 "\insn  r16, r16, 0"
  insn_32 "\insn  r16, r16, 0x1f"
.endm

.text
/*
 * bitclr rD,rA,BN5 -> bitclr! rD,BN5
 * bitset rD,rA,BN5 -> bitset! rD,BN5
 * bittgl rD,rA,BN5 -> bittgl! rD,BN5
 */
_bit_op_pattern "bitclr", "bitclr.c"
_bit_op_pattern "bitset", "bitset.c"
_bit_op_pattern "bittgl", "bittgl.c"

/* bittst.c rA,BN5 <-> bittst! rD,BN5" */
insn_32 "bittst.c r0,  0"
insn_32 "bittst.c r0,  0x1f"
insn_32 "bittst.c r15, 0"
insn_32 "bittst.c r15, 0x1f"

tran_16_32 "bittst! r0,0", "bittst.c r0,0"

/* shouldn't alter */
insn_32 "bittst.c r16, 0"
insn_32 "bittst.c r16, 0x1f"
