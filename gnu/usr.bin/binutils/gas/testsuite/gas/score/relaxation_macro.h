/*
 * macros for S+core 3 instruction relaxation
 *
 * partial copyed from testpatterns for S+core 7
 *
 * Author: libin
 */

.macro _tran insn1 insn2
.balign 2
  .irp i1,"\insn1", "\insn2"
    .irp i2,"\insn1", "\insn2"
      \i1
      \i2
    .endr
  .endr
.endm

/* insn32/insn16 may include special characters, for example, blank character */
.macro tran_16_32 insn16 insn32
  _tran "\insn16", "\insn32"
.endm

.macro insn_16 insn16
.balign 2
  \insn16
.endm

.macro insn_32 insn32
.balign 2
  \insn32
.endm
