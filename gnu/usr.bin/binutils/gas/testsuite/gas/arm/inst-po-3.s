.syntax unified
.arch armv7a
.thumb

@ it eq
@ mov r1, r9
@ mov r1, r9
moveq r1, r9
.inst 0x4649
.inst 0x4649

.word 0x1234

@ ite eq
@ moveq r1, r9
@ movne r1, r9
.inst 0xbf0b + 1, 0x4649, 0x4649
