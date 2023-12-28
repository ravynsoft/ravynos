.syntax unified

@ Loads, ARM ================================================================
.arm

@ LDR (immediate, ARM)
@ LDR (literal)
@No unpredictable or undefined combinations.

@ LDR (register)
ldr r0,[r1,pc, LSL #2]			@ Unpredictable
ldr r0,[r1,pc, LSL #2]!			@ ditto
ldr r0,[r1],pc, LSL #2			@ ditto
ldr r0,[pc,r1, LSL #2]!			@ ditto
ldr r0,[pc],r1, LSL #2			@ ditto

@ LDRB (immediate, ARM)
ldrb pc,[r0,#4]				@ Unpredictable
ldrb pc,[r0],#4				@ ditto
ldrb pc,[r0,#4]!			@ ditto

@ LDRB (literal)
ldrb pc, label				@ Unpredictable
ldrb pc,[pc,#-0]			@ ditto

@ LDRB (register)
ldrb pc,[r0,r1, LSL #2]			@ Unpredictable
ldrb pc,[r0,r1, LSL #2]!		@ ditto
ldrb pc,[r0],r1, LSL #2			@ ditto
ldrb r0,[r1,pc, LSL #2]			@ ditto
ldrb r0,[r1,pc, LSL #2]!		@ ditto
ldrb r0,[r1],pc, LSL #2			@ ditto
ldrb r0,[pc,r1, LSL #2]!		@ ditto
ldrb r0,[pc],r1, LSL #2			@ ditto

@ LDRBT
ldrbt pc,[r0],#4			@ Unpredictable
ldrbt r0,[pc],#4			@ ditto
ldrbt pc,[r0],r1, LSL #4		@ ditto
ldrbt r0,[pc],r1, LSL #4		@ ditto
ldrbt r0,[r1],pc, LSL #4		@ ditto

@ LDRD (immediate)
ldrd r0,pc,[r1,#4]			@ Unpredictable
ldrd r0,pc,[r1],#4			@ ditto
ldrd r0,pc,[r1,#4]!			@ ditto

@ LDRD (literal)
ldrd r0,pc, label			@ Unpredictable
ldrd r0,pc,[PC,#-0]			@ ditto

@ LDRD (register)
ldrd r0,pc,[r1,r2]			@ Unpredictable
ldrd r0,pc,[r1,r2]!			@ ditto
ldrd r0,pc,[r1],r2			@ ditto
ldrd r0,r1,[r2,pc]			@ ditto
ldrd r0,r1,[r2,pc]!			@ ditto
ldrd r0,r1,[r2],pc			@ ditto
ldrd r0,r1,[pc,r2]!			@ ditto
ldrd r0,r1,[pc],r2			@ ditto

@ LDREX
ldrex pc,[r0]				@ Unpredictable
ldrex r0,[pc]				@ ditto

@ LDREXB
ldrexb pc,[r0]				@ Unpredictable
ldrexb r0,[pc]				@ ditto

@ LDREXD
ldrexd r0,r1,[pc]			@ Unpredictable

@ LDREXH
ldrexh pc,[r0]				@ Unpredictable
ldrexh r0,[pc]				@ ditto

@ LDRH (immediate, ARM)
ldrh pc,[r0,#4]				@ Unpredictable
ldrh pc,[r0],#4				@ ditto
ldrh pc,[r0,#4]!			@ ditto

@ LDRH (literal)
ldrh pc, label				@ Unpredictable
ldrh pc,[pc,#-0]			@ ditto

@ LDRH (register)
ldrh pc,[r0,r1]				@ Unpredictable
ldrh pc,[r0,r1]!			@ ditto
ldrh pc,[r0],r1				@ ditto
ldrh r0,[r1,pc]				@ ditto
ldrh r0,[r1,pc]!			@ ditto
ldrh r0,[r1],pc				@ ditto
ldrh r0,[pc,r1]!			@ ditto
ldrh r0,[pc],r1				@ ditto

@ LDRHT
ldrht pc, [r0], #4			@ Unpredictable
ldrht r0, [pc], #4			@ ditto
ldrht pc, [r0], r1			@ ditto
ldrht r0, [pc], r1			@ ditto
ldrht r0, [r1], pc			@ ditto

@ LDRSB (immediate)
ldrsb pc,[r0,#4]			@ Unpredictable
ldrsb pc,[r0],#4			@ ditto
ldrsb pc,[r0,#4]!			@ ditto

@ LDRSB (literal)
ldrsb pc, label				@ Unpredictable
ldrsb pc,[pc,#-0]			@ ditto

@ LDRSB (register)
ldrsb pc,[r0,r1]			@ Unpredictable
ldrsb pc,[r0,r1]!			@ ditto
ldrsb pc,[r0],r1			@ ditto
ldrsb r0,[r1,pc]			@ ditto
ldrsb r0,[r1,pc]!			@ ditto
ldrsb r0,[r1],pc			@ ditto
ldrsb r0,[pc,r1]!			@ ditto
ldrsb r0,[pc],r1			@ ditto

@ LDRSBT
ldrsbt pc, [r0], #4			@ Unpredictable
ldrsbt r0, [pc], #4			@ ditto
ldrsbt pc, [r0], r1			@ ditto
ldrsbt r0, [pc], r1			@ ditto
ldrsbt r0, [r1], pc			@ ditto

@ LDRSH (immediate)
ldrsh pc,[r0,#4]			@ Unpredictable
ldrsh pc,[r0],#4			@ ditto
ldrsh pc,[r0,#4]!			@ ditto

@ LDRSH (literal)
ldrsh pc, label				@ Unpredictable
ldrsh pc,[pc,#-0]			@ ditto

@ LDRSH (register)
ldrsh pc,[r0,r1]			@ Unpredictable
ldrsh pc,[r0,r1]!			@ ditto
ldrsh pc,[r0],r1			@ ditto
ldrsh r0,[r1,pc]			@ ditto
ldrsh r0,[r1,pc]!			@ ditto
ldrsh r0,[r1],pc			@ ditto
ldrsh r0,[pc,r1]!			@ ditto
ldrsh r0,[pc],r1			@ ditto

@ LDRSHT
ldrsht pc, [r0], #4			@ Unpredictable
ldrsht r0, [pc], #4			@ ditto
ldrsht pc, [r0], r1			@ ditto
ldrsht r0, [pc], r1			@ ditto
ldrsht r0, [r1], pc			@ ditto

@ LDRT
ldrt pc, [r0], #4			@ Unpredictable
ldrt r0, [pc], #4			@ ditto
ldrt pc,[r0],r1, LSL #4			@ ditto
ldrt r0,[pc],r1, LSL #4			@ ditto
ldrt r0,[r1],pc, LSL #4			@ ditto


@ Stores, ARM ================================================================

@ STR (immediate, ARM)
str r0,[pc],#4				@ Unpredictable
str r0,[pc,#4]!				@ ditto

@ STR (register)
str r0,[r1,pc, LSL #4]			@ Unpredictable
str r0,[r1,pc, LSL #4]!			@ ditto
str r0,[r1],pc, LSL #4			@ ditto

@ STRB (immediate, ARM)
strb pc,[r0,#4]				@ Unpredictable
strb pc,[r0],#4				@ ditto
strb pc,[r0,#4]!			@ ditto
strb r0,[pc],#4				@ ditto
strb r0,[pc,#4]!			@ ditto

@ STRB (register)
strb pc,[r0,r1, LSL #4]			@ Unpredictable
strb pc,[r0,r1, LSL #4]!		@ ditto
strb pc,[r0],r1, LSL #4			@ ditto
strb r1,[r0,pc, LSL #4]			@ ditto
strb r1,[r0,pc, LSL #4]!		@ ditto
strb r1,[r0],pc, LSL #4			@ ditto
strb r0,[pc,r1, LSL #4]!		@ ditto
strb r0,[pc],r1, LSL #4			@ ditto

@ STRBT
strbt pc,[r0],#4			@ Unpredictable
strbt r0,[pc],#4			@ ditto
strbt pc,[r0],r1, LSL #4		@ ditto
strbt r0,[pc],r1, LSL #4		@ ditto
strbt r0,[r1],pc, LSL #4		@ ditto

@ STRD (immediate)
strd r0,pc,[r1,#4]			@ ditto
strd r0,pc,[r1],#4			@ ditto
strd r0,pc,[r1,#4]!			@ ditto
strd r0,r1,[pc],#4			@ ditto
strd r0,r1,[pc,#4]!			@ ditto

@STRD (register)
strd r0,pc,[r1,r2]			@ Unpredictable
strd r0,pc,[r1,r2]!			@ ditto
strd r0,pc,[r1],r2			@ ditto
strd r0,r1,[r2,pc]			@ ditto
strd r0,r1,[r2,pc]!			@ ditto
strd r0,r1,[r2],pc			@ ditto
strd r0,r1,[pc,r2]!			@ ditto
strd r0,r1,[pc],r2			@ ditto

@ STREX
strex pc,r0,[r1]			@ Unpredictable
strex r0,pc,[r1]			@ ditto
strex r0,r1,[pc]			@ ditto

@ STREXB
strexb pc,r0,[r1]			@ Unpredictable
strexb r0,pc,[r1]			@ ditto
strexb r0,r1,[pc]			@ ditto

@ STREXD
strexd pc,r0,r1,[r2]			@ Unpredictable
strexd r0,r1,r2,[pc]			@ ditto

@ STREXH
strexh pc,r0,[r1]			@ Unpredictable
strexh r0,pc,[r1]			@ ditto
strexh r0,r1,[pc]			@ ditto

@ STRH (immediate, ARM)
strh pc,[r0,#4]				@ Unpredictable
strh pc,[r0],#4				@ ditto
strh pc,[r0,#4]!			@ ditto
strh r0,[pc],#4				@ ditto
strh r0,[pc,#4]!			@ ditto

@ STRH (register)
strh pc,[r0,r1]				@ Unpredictable
strh pc,[r0,r1]!			@ ditto
strh pc,[r0],r1				@ ditto
strh r0,[r1,pc]				@ ditto
strh r0,[r1,pc]!			@ ditto
strh r0,[r1],pc				@ ditto
strh r0,[pc,r1]!			@ ditto
strh r0,[pc],r1				@ ditto

@ STRHT
strht pc, [r0], #4			@ Unpredictable
strht r0, [pc], #4			@ ditto
strht pc, [r0], r1			@ ditto
strht r0, [pc], r1			@ ditto
strht r0, [r1], pc			@ ditto

@ STRT
strt r0, [pc], #4			@ Unpredictable
strt r0, [pc],r1, LSL #4		@ ditto
strt r0, [r1],pc, LSL #4		@ ditto

@ ============================================================================

.label:
ldr r0, [r1]
