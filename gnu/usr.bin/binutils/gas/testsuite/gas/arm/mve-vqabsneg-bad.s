.syntax unified
.thumb
vqabs.u8 q0, q1
vqneg.u16 q0, q1
vqabs.s64 q0, q1
vqnegs.s64 q0, q1

.irp op, vqabs, vqneg
.irp cond, eq, ne, gt, ge, lt, le
it \cond
\op\().s16 q0, q1
.endr
.endr


it eq
vqabseq.s32 q0, q1
vqabseq.s32 q0, q1
vqabseq.s32 q0, q1
vqabst.s32 q0, q1
vpst
vqabs.s32 q0, q1
it eq
vqnegeq.s32 q0, q1
vqnegeq.s32 q0, q1
vqnegeq.s32 q0, q1
vqnegt.s32 q0, q1
vpst
vqneg.s32 q0, q1
