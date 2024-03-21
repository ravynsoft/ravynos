.syntax unified
.thumb

.irp op1, r13, r15
.irp op2 8, 16, 32, 64, s8, u16, f32
vctp.\op2 \op1
.endr
.endr

.irp op1, r0, r1, r2, r4, r8
.irp op2 8, 16, 32, 64, f32
vctpt.\op2 \op1
.endr
.endr
