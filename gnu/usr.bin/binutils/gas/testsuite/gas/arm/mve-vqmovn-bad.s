.syntax unified
.thumb
vqmovnt.s8 q0, q1
vqmovnt.s64 q0, q1
vqmovnt.i16 q0, q1
vqmovnb.u8 q0, q1
vqmovnb.u64 q0, q1
vqmovnb.i16 q0, q1
vqmovunt.s8 q0, q1
vqmovunt.s64 q0, q1
vqmovunt.i16 q0, q1
vqmovunb.s8 q0, q1
vqmovunb.s64 q0, q1
vqmovunb.i16 q0, q1
vqmovunt.u16 q0, q1
vqmovunt.u32 q0, q1
vqmovunb.u16 q0, q1
vqmovunb.u32 q0, q1

.irp op, vqmovnt, vqmovnb, vqmovunt, vqmovunb
.irp cond, eq, ne, gt, ge, lt, le

it \cond
\op\().s16 q0, q1

.endr
.endr


it eq
vqmovnteq.s16 q0, q1
vqmovnteq.s16 q0, q1
vpst
vqmovnteq.s16 q0, q1
vqmovntt.s16 q0, q1
vpst
vqmovnt.s16 q0, q1
it eq
vqmovnbeq.s16 q0, q1
vqmovnbeq.s16 q0, q1
vpst
vqmovnbeq.s16 q0, q1
vqmovnbt.s16 q0, q1
vpst
vqmovnb.s16 q0, q1
it eq
vqmovunteq.s16 q0, q1
vqmovunteq.s16 q0, q1
vpst
vqmovunteq.s16 q0, q1
vqmovuntt.s16 q0, q1
vpst
vqmovunt.s16 q0, q1
it eq
vqmovunbeq.s16 q0, q1
vqmovunbeq.s16 q0, q1
vpst
vqmovunbeq.s16 q0, q1
vqmovunbt.s16 q0, q1
vpst
vqmovunb.s16 q0, q1
