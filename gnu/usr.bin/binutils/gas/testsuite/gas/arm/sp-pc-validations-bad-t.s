.syntax unified
@ Enable Thumb mode
.thumb
.macro it_test opcode operands:vararg
itt eq
\opcode\()eq r15, \operands
moveq r0, r0
.endm

.macro it_testw opcode operands:vararg
itt eq
\opcode\()eq.w r15, \operands
moveq r0, r0
.endm

.macro LOAD operands:vararg
it_test ldr, \operands
.endm

.macro LOADw operands:vararg
it_testw ldr, \operands
.endm

@ Loads ===============================================================

@ LDR (register)
LOAD  [r0]
LOAD  [r0,#0]
LOAD  [sp]
LOAD  [sp,#0]
LOADw [r0]
LOADw [r0,#0]
LOAD  [r0,#-4]
LOAD  [r0],#4
LOAD  [r0,#0]!

@ LDR (literal)
LOAD  label
LOADw label
LOADw [pc, #-0]

@ LDR (register)
LOAD  [r0, r1]
LOADw [r0, r1]
LOADw [r0, r1, LSL #2]

@ LDRB (immediate, Thumb)
ldrb	pc, [r0,#4]			@ low reg
@ldrb	r0, [pc,#4]			@ ALLOWED!
ldrb.w	sp, [r0,#4]			@ Unpredictable
ldrb.w	pc, [r0,#4]			@ => PLD
ldrb	pc, [r0, #-4]			@ => PLD
@ LDRB<c><q> <Rt>, [<Rn>, #+<imm>]	=> See LDRBT
ldrb	pc, [r0],#4			@ BadReg
ldrb	sp, [r0],#4			@ ditto
ldrb	pc,[r0,#4]!			@ ditto
ldrb	sp,[r0,#4]!			@ ditto

@ LDRB (literal)
ldrb	pc,label			@ => PLD
ldrb	pc,[PC,#-0]			@ => PLD (special case)
ldrb	sp,label			@ Unpredictable
ldrb	sp,[PC,#-0]			@ ditto

@ LDRB (register)
ldrb	pc,[r0,r1]			@ low reg
ldrb	r0,[pc,r1]			@ ditto
ldrb	r0,[r1,pc]			@ ditto
ldrb.w	pc,[r0,r1,LSL #1]		@ => PLD
ldrb.w	sp,[r0,r1]			@ Unpredictable
ldrb.w	r2,[r0,pc,LSL #2]		@ BadReg
ldrb.w	r2,[r0,sp,LSL #2]		@ ditto

@ LDRBT
ldrbt	pc, [r0, #4]			@ BadReg
ldrbt	sp, [r0, #4]			@ ditto

@ LDRD (immediate)
ldrd pc, r0, [r1]			@ BadReg
ldrd sp, r0, [r1]			@ ditto
ldrd r12, [r1]				@ ditto
ldrd r14, [r1]				@ ditto
ldrd r0, pc, [r1]			@ ditto
ldrd r0, sp, [r1]			@ ditto
ldrd pc, r0, [r1], #4			@ ditto
ldrd sp, r0, [r1], #4			@ ditto
ldrd r0, pc, [r1], #4			@ ditto
ldrd r0, sp, [r1], #4			@ ditto
ldrd r12, [r1], #4			@ ditto
ldrd r14, [r1], #4			@ ditto
ldrd pc, r0, [r1, #4]!			@ ditto
ldrd sp, r0, [r1, #4]!			@ ditto
ldrd r0, pc, [r1, #4]!			@ ditto
ldrd r0, sp, [r1, #4]!			@ ditto
ldrd r12, [r1, #4]!			@ ditto
ldrd r14, [r1, #4]!			@ ditto

@ LDRD (literal)
ldrd pc, r0, label			@ BadReg
ldrd sp, r0, label			@ ditto
ldrd r0, pc, label			@ ditto
ldrd r0, sp, label			@ ditto
ldrd pc, r0, [pc, #-0]			@ ditto
ldrd sp, r0, [pc, #-0]			@ ditto
ldrd r0, pc, [pc, #-0]			@ ditto
ldrd r0, sp, [pc, #-0]			@ ditto

@ LDRD (register): ARM only

@ LDREX/B/D/H
ldrex  pc, [r0]				@ BadReg
ldrex  sp, [r0]				@ ditto
ldrex  r0, [pc]				@ Unpredictable
ldrexb pc, [r0]				@ BadReg
ldrexb sp, [r0]				@ ditto
ldrexb r0, [pc]				@ Unpredictable
ldrexd pc, r0, [r1]			@ BadReg
ldrexd sp, r0, [r1]			@ ditto
ldrexd r0, pc, [r1]			@ ditto
ldrexd r0, sp, [r1]			@ ditto
ldrexd r0, r1, [pc]			@ Unpredictable
ldrexh pc, [r0]				@ BadReg
ldrexh sp, [r0]				@ ditto
ldrexh r0, [pc]				@ Unpredictable

@ LDRH (immediate)
ldrh pc, [r0]				@ low reg
ldrh pc, [r0, #4]			@ ditto
@ldrh r0, [pc]				@ ALLOWED!
@ldrh r0, [pc, #4]			@ ditto
ldrh.w pc, [r0]				@ => Unallocated memory hints
ldrh.w pc, [r0, #4]			@ ditto
ldrh.w sp, [r0]				@ Unpredictable
ldrh.w sp, [r0, #4]			@ ditto
ldrh pc, [r0, #-3]			@ => Unallocated memory hint
@ LDRH<c><q> <Rt>, [<Rn>, #+<imm>]	=> See LDRHT
ldrh pc,[r0],#4				@ BadReg
ldrh sp,[r0],#4				@ ditto
ldrh pc,[r0,#4]!			@ ditto
ldrh sp,[r0,#4]!			@ ditto

@ LDRH (literal)
ldrh pc, label				@ Unallocated memory hint
ldrh pc, [pc, #-0]			@ ditto
ldrh sp, label				@ Unpredictable
ldrh sp, [pc, #-0]			@ ditto

@ LDRH (register)
ldrh pc, [r0, r1]			@ low reg
ldrh r0, [pc, r1]			@ ditto
ldrh r0, [r1, pc]			@ ditto
ldrh.w pc,[r0,r1,LSL #1]		@ => Unallocated memory hints
ldrh.w sp,[r0,r1,LSL #1]		@ Unpredictable
ldrh.w r2,[r0,pc,LSL #1]		@ ditto
ldrh.w r2,[r0,sp,LSL #1]		@ ditto

@ LDRHT
ldrht pc, [r0, #4]			@ BadReg
ldrht sp, [r0, #4]			@ ditto

@ LDRSB (immediate)
ldrsb pc, [r0, #4]			@ => PLI
@ldrsb r0, [pc, #4]			=> LDRSB (literal)
ldrsb sp, [r0, #4]			@ Unpredictable
ldrsb pc, [r0, #-4]			@ => PLI
ldrsb sp,[r0,#-4]			@ BadReg
ldrsb pc,[r0],#4			@ ditto
ldrsb sp,[r0],#4			@ ditto
ldrsb pc,[r0,#4]!			@ ditto
ldrsb sp,[r0,#4]!			@ ditto

@ LDRSB (literal)
ldrsb pc, label				@ => PLI
ldrsb pc, [pc, #-0]			@ => PLI
ldrsb sp, label				@ Unpredictable
ldrsb sp, [pc, #-0]			@ ditto

@ LDRSB (register)
ldrsb pc, [r0, r1]			@ low reg
ldrsb r0, [pc, r1]			@ ditto
ldrsb r0, [r1, pc]			@ ditto
ldrsb.w pc, [r0, r1, LSL #2]		@ => PLI
@ldrsb.w r0, [pc, r0, LSL #2]		=> LDRSB (literal)
ldrsb.w sp, [r0, r1, LSL #2]		@ Unpredictable
ldrsb.w r2, [r0, pc, LSL #2]		@ ditto
ldrsb.w r2, [r0, sp, LSL #2]		@ ditto

@ LDRSBT
@ldrsbt r0, [pc, #4]			=> LDRSB (literal)
ldrsbt pc, [r0, #4]			@ BadReg
ldrsbt sp, [r0, #4]			@ ditto

@ LDRSH (immediate)
@ldrsh r0,[pc,#4]			=> LDRSH (literal)
ldrsh pc,[r0,#4]			@ => Unallocated memory hints
ldrsh sp,[r0,#4]			@ Unpredictable
ldrsh pc, [r0, #-4]			@ => Unallocated memory hints
ldrsh pc,[r0],#4			@ BadReg
ldrsh pc,[r0,#4]!			@ ditto
ldrsh sp,[r0,#-4]			@ ditto
ldrsh sp,[r0],#4			@ ditto
ldrsh sp,[r0,#4]!			@ ditto

@ LDRSH (literal)
ldrsh pc, label				@ => Unallocated memory hints
ldrsh sp, label				@ Unpredictable
ldrsh sp, [pc,#-0]			@ ditto

@ LDRSH (register)
ldrsh pc,[r0,r1]			@ low reg
ldrsh r0,[pc,r1]			@ ditto
ldrsh r0,[r1,pc]			@ ditto
@ldrsh.w r0,[pc,r1,LSL #3]		=> LDRSH (literal)
ldrsh.w pc,[r0,r1,LSL #3]		@ => Unallocated memory hints
ldrsh.w sp,[r0,r1,LSL #3]		@ Unpredictable
ldrsh.w r0,[r1,sp,LSL #3]		@ BadReg
ldrsh.w r0,[r1,pc,LSL #3]		@ ditto

@ LDRSHT
@ldrsht r0,[pc,#4]			=> LDRSH (literal)
ldrsht pc,[r0,#4]			@ BadReg
ldrsht sp,[r0,#4]			@ ditto

@ LDRT
@ldrt r0,[pc,#4]			=> LDR (literal)
ldrt pc,[r0,#4]				@ BadReg
ldrt sp,[r0,#4]				@ ditto

@ Stores ==============================================================

@ STR (immediate, Thumb)
str pc, [r0, #4]			@ Unpredictable
str.w r0, [pc, #4]			@ Undefined
str r0, [pc, #-4]			@ ditto
str r0, [pc], #4			@ ditto
str r0, [pc, #4]!			@ ditto

@ STR (register)
str.w r0,[pc,r1]			@ Undefined
str.w r0,[pc,r1,LSL #2]			@ ditto
@str.w pc,[r0,r1{,LSL #<imm2>}]		@ Unpredictable
@str.w r1,[r0,sp{,LSL #<imm2>}]		@ ditto
@str.w r1,[r0,pc{,LSL #<imm2>}]		@ ditto

@ STRB (immediate, Thumb)
strb.w r0,[pc,#4]			@ Undefined
strb.w pc,[r0,#4]			@ Unpredictable
strb.w sp,[r0,#4]			@ ditto
strb r0,[pc,#-4]			@ Undefined
strb r0,[pc],#4				@ ditto
strb r0,[pc,#4]!			@ ditto
strb pc,[r0,#-4]			@ Unpredictable
strb pc,[r0],#4				@ ditto
strb pc,[r0,#4]!			@ ditto
strb sp,[r0,#-4]			@ ditto
strb sp,[r0],#4				@ ditto
strb sp,[r0,#4]!			@ ditto

@ STRB (register)
strb.w r0,[pc,r1]			@ Undefined
strb.w r0,[pc,r1,LSL #2]		@ ditto
strb.w pc,[r0,r1]			@ Unpredictable
strb.w pc,[r0,r1,LSL #2]		@ ditto
strb.w sp,[r0,r1]			@ ditto
strb.w sp,[r0,r1,LSL #2]		@ ditto
strb.w r0,[r1,pc]			@ ditto
strb.w r0,[r1,pc,LSL #2]		@ ditto
strb.w r0,[r1,sp]			@ ditto
strb.w r0,[r1,sp,LSL #2]		@ ditto

@ STRBT
strbt r0,[pc,#4]			@ Undefined
strbt pc,[r0,#4]			@ Unpredictable
strbt sp,[r0,#4]			@ ditto

@ STRD (immediate)
strd r0,r1,[pc,#4]			@ Unpredictable
strd r0,r1,[pc],#4			@ ditto
strd r0,r1,[pc,#4]!			@ ditto
strd pc,r0,[r1,#4]			@ ditto
strd pc,r0,[r1],#4			@ ditto
strd pc,r0,[r1,#4]!			@ ditto
strd sp,r0,[r1,#4]			@ ditto
strd sp,r0,[r1],#4			@ ditto
strd sp,r0,[r1,#4]!			@ ditto
strd r0,pc,[r1,#4]			@ ditto
strd r0,pc,[r1],#4			@ ditto
strd r0,pc,[r1,#4]!			@ ditto
strd r0,sp,[r1,#4]			@ ditto
strd r0,sp,[r1],#4			@ ditto
strd r0,sp,[r1,#4]!			@ ditto

@ STRD (register)
@No thumb.

@ STREX
strex pc,r0,[r1]			@ Unpredictable
strex pc,r0,[r1,#4]			@ ditto
strex sp,r0,[r1]			@ ditto
strex sp,r0,[r1,#4]			@ ditto
strex r0,pc,[r1]			@ ditto
strex r0,pc,[r1,#4]			@ ditto
strex r0,sp,[r1]			@ ditto
strex r0,sp,[r1,#4]			@ ditto
strex r0,r1,[pc]			@ ditto
strex r0,r1,[pc,#4]			@ ditto

@ STREXB
strexb pc,r0,[r1]			@ Unpredictable
strexb sp,r0,[r1]			@ ditto
strexb r0,pc,[r1]			@ ditto
strexb r0,sp,[r1]			@ ditto
strexb r0,r1,[pc]			@ ditto

@ STREXD
strexd pc,r0,r1,[r2]			@ Unpredictable
strexd sp,r0,r1,[r2]			@ ditto
strexd r0,pc,r1,[r2]			@ ditto
strexd r0,sp,r1,[r2]			@ ditto
strexd r0,r1,pc,[r2]			@ ditto
strexd r0,r1,sp,[r2]			@ ditto
strexd r0,r1,r2,[pc]			@ ditto

@ STREXH
strexh pc,r0,[r1]			@ Unpredictable
strexh sp,r0,[r1]			@ ditto
strexh r0,pc,[r1]			@ ditto
strexh r0,sp,[r1]			@ ditto
strexh r0,r1,[pc]			@ ditto

@ STRH (immediate, Thumb)
strh.w r0,[pc]				@ Undefined
strh.w r0,[pc,#4]			@ ditto
strh r0,[pc,#-4]			@ ditto
strh r0,[pc],#4				@ ditto
strh r0,[pc,#4]!			@ ditto

@ STRH (register)
strh.w r0,[pc,r1]			@ Undefined
strh.w r0,[pc,r1,LSL #2]		@ ditto
strh.w pc,[r0,#4]			@ Unpredictable
strh.w pc,[r0]				@ ditto
strh.w sp,[r0,#4]			@ ditto
strh.w sp,[r0]				@ ditto
strh pc,[r0,#-4]			@ ditto
strh pc,[r0],#4				@ ditto
strh pc,[r0,#4]!			@ ditto
strh sp,[r0,#-4]			@ ditto
strh sp,[r0],#4				@ ditto
strh sp,[r0,#4]!			@ ditto
strh.w pc,[r0,r1]			@ ditto
strh.w sp,[r0,r1]			@ ditto
strh.w r0,[r1,pc]			@ ditto
strh.w r0,[r1,sp]			@ ditto
strh.w pc,[r0,r1,LSL #2]		@ ditto
strh.w sp,[r0,r1,LSL #2]		@ ditto
strh.w r0,[r1,pc,LSL #2]		@ ditto
strh.w r0,[r1,sp,LSL #2]		@ ditto

@ STRHT
strht r0,[pc,#4]			@ Undefined
strht pc,[r0,#4]			@ Unpredictable
strht sp,[pc,#4]			@ ditto

@ STRT
strt r0,[pc,#4]				@ Undefined
strt pc,[r0,#4]				@ Unpredictable
strt sp,[r0,#4]				@ ditto

@ ============================================================================

.label:
ldr r0, [r1]
