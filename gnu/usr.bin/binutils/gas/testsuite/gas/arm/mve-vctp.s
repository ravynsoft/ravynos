.syntax unified
.thumb

.irp op1, r0, r1, r2, r4, r8
.irp op2 8, 16, 32, 64
vctp.\op2 \op1
.endr
.endr

.irp op1, r0, r1, r2, r4, r8
.irp op2 8, 16, 32, 64
vpst
vctpt.\op2 \op1
.endr
.endr
