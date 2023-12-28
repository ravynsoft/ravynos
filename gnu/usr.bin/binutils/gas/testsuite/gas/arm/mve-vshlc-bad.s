.syntax unified
.thumb
vshlc q0, r1, #0
vshlc q0, r1, #33
vshlc q0, sp, #1
vshlc q0, pc, #1

.irp cond, eq, ne, gt, ge, lt, le

it \cond
vshlc q0, r1, #2

.endr

it eq
vshlceq q0, r1, #2
vshlceq q0, r1, #2
vpst
vshlceq q0, r1, #2
vshlct q0, r1, #2
vpst
vshlc q0, r1, #2
