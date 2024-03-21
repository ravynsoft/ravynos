.syntax unified
.thumb

vqrshrnt.s8 q0, q1, #1
vqrshrnt.s64 q0, q1, #1
vqrshrnt.s16 q0, q1, #0
vqrshrnt.s16 q0, q1, #9
vqrshrnt.s32 q0, q1, #0
vqrshrnt.s32 q0, q1, #17
vqrshrnb.s8 q0, q1, #1
vqrshrnb.s64 q0, q1, #1
vqrshrnb.s16 q0, q1, #0
vqrshrnb.s16 q0, q1, #9
vqrshrnb.s32 q0, q1, #0
vqrshrnb.s32 q0, q1, #17
vqrshrunt.s8 q0, q1, #1
vqrshrunt.s64 q0, q1, #1
vqrshrunt.s16 q0, q1, #0
vqrshrunt.s16 q0, q1, #9
vqrshrunt.s32 q0, q1, #0
vqrshrunt.s32 q0, q1, #17
vqrshrunt.u16 q0, q1, #1
vqrshrunb.s8 q0, q1, #1
vqrshrunb.s64 q0, q1, #1
vqrshrunb.s16 q0, q1, #0
vqrshrunb.s16 q0, q1, #9
vqrshrunb.s32 q0, q1, #0
vqrshrunb.s32 q0, q1, #17
vqrshrunb.u16 q0, q1, #1

.irp op, vqrshrnt, vqrshrnb, vqrshrunt, vqrshrunb
.irp cond, eq, ne, gt, ge, lt, le

it \cond
\op\().s16 q0, q0, #1

.endr
.endr

it eq
vqrshrnteq.s16 q0, q1, #1
vqrshrnteq.s16 q0, q1, #1
vpst
vqrshrnteq.s16 q0, q1, #1
vqrshrntt.s16 q0, q1, #1
vpst
vqrshrnt.s16 q0, q1, #1
it eq
vqrshrnbeq.s16 q0, q1, #1
vqrshrnbeq.s16 q0, q1, #1
vpst
vqrshrnbeq.s16 q0, q1, #1
vqrshrnbt.s16 q0, q1, #1
vpst
vqrshrnb.s16 q0, q1, #1
it eq
vqrshrunteq.s16 q0, q1, #1
vqrshrunteq.s16 q0, q1, #1
vpst
vqrshrunteq.s16 q0, q1, #1
vqrshruntt.s16 q0, q1, #1
vpst
vqrshrunt.s16 q0, q1, #1
it eq
vqrshrunbeq.s16 q0, q1, #1
vqrshrunbeq.s16 q0, q1, #1
vpst
vqrshrunbeq.s16 q0, q1, #1
vqrshrunbt.s16 q0, q1, #1
vpst
vqrshrunb.s16 q0, q1, #1
