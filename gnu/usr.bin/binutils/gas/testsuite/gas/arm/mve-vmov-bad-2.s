.syntax unified
.thumb
vmov r0, r0, q0[2], q0[0]
vmov sp, r0, q0[2], q0[0]
vmov r0, sp, q0[2], q0[0]
vmov pc, r0, q0[2], q0[0]
vmov r0, pc, q0[2], q0[0]
vmov q0[2], q0[0], sp, r0
vmov q0[2], q0[0], r0, sp
vmov q0[2], q0[0], pc, r0
vmov q0[2], q0[0], r0, pc
