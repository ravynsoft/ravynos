.syntax unified
.arch armv7a
.arm
@ movne r1,r9
.inst 0x11a01009

.thumb

@ ite eq
@ moveq r1, r9
@ movne r1, r9
.inst 0xbf0b + 1, 0x4649
.inst 0x4649

.word 0x1234

@ ite eq
@ moveq r1, r9
@ movne r1, r9
.inst.n 0xbf0b + 1, 0x4649, 0x4649

.inst.n 0x4649

@ mov.w r1, r9
@ mov.w r1, r9
.inst 0xea4f0109
.inst.w 0xea4f0109
