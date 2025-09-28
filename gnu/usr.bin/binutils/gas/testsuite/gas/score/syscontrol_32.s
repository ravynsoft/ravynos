/*
 * tests for system control instruction relaxation
 *
 * Author: libin
 */

.include "relaxation_macro.h"

.text
/* sdbbp Imm5 -> sdbbp! Imm5 */
insn_32 "sdbbp 0"
insn_32 "sdbbp 31"

tran_16_32 "sdbbp! 0", "sdbbp 0"

/* nop -> nop! */
insn_32 "nop"

tran_16_32 "nop!", "nop"
