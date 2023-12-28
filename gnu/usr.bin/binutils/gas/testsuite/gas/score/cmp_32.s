/*
 * tests for compare instruction relaxation
 *
 * Author: libin
 */

.include "relaxation_macro.h"

.text 
/* cmp.c rA,rB -> cmp! rA,rB */
insn_32 "cmp.c r0, r15"

tran_16_32 "cmp! r0, r15", "cmp.c r0, r15"

/* shouln't alter */
insn_32 "cmp.c r0,  r16"
insn_32 "cmp.c r16, r0"
insn_32 "cmp.c r16, r31"

/* cmpi.c rD,SImm16 -> cmpi! rD,SImm5 */
insn_32 "cmpi.c r0,  -16"
insn_32 "cmpi.c r0,  15"
insn_32 "cmpi.c r15, -16"
insn_32 "cmpi.c r15, 15"

tran_16_32 "cmpi! r0, -16", "cmpi.c r0, -16"

/* shouldn't alter */
insn_32 "cmpi.c r16, -16"
insn_32 "cmpi.c r31, 15"
insn_32 "cmpi.c r0, -17"
insn_32 "cmpi.c r15, 16"
insn_32 "cmpi.c r16, 16"
