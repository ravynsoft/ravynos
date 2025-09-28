.syntax unified
.thumb

.label_back:
.irp data, 8, 16, 32, 64
.irp op2, r0, r1, r2, r4, r7, r8, r10, r12, r14
wlstp.\data lr, \op2, .label
.endr
.endr
.irp data, 8, 16, 32, 64
.irp op2, r0, r1, r2, r4, r7, r8, r10, r12, r14
dlstp.\data lr, \op2
.endr
.endr
le lr, .label_back
le .label_back
letp lr, .label_back
lctp
.irp cond, eq, ne, gt, ge, lt, le
it \cond
lctp\cond
.endr
.label:
