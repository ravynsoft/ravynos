.syntax unified
.arch armv7a
.arm
.L1:

moveq r1, r9
.inst .L1

.arm
.inst.w 1
.inst.n 1

.thumb
.inst 0xf000
.inst.n 1<<31

