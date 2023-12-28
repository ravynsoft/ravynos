/*
 * tests for mv instruction relaxation
 *
 * Author: libin
 */

.include "relaxation_macro.h"

.text
/* mv rD,rA -> mv! rD,rA */
insn_32 "mv r0, r15"

tran_16_32 "mv! r0, r15", "mv r0, r15"

/* shouldn't alter */
insn_32 "mv r16, r15"
insn_32 "mv r0,  r16"
insn_32 "mv r16, r16"
