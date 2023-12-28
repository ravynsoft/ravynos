.syntax unified
.thumb
vmvn.i16 d0, d1
vmvn.i32 q0, #0x1ef

.irp lastop, q1, #1
.irp cond, eq, ne, gt, ge, lt, le
it \cond
vmvn.i16 q0, \lastop
.endr
.endr


it eq
vmvneq q0, q1
vmvneq q0, q1
vpst
vmvneq q0, q1
vmvnt q0, q1
vpst
vmvn q0, q1
