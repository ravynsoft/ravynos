.label_back:
.syntax unified
.thumb

.irp cond, eq, ne, gt, ge, lt, le
it \cond
wlstp.8 lr, r0, .label
.endr

.irp cond, eq, ne, gt, ge, lt, le
it \cond
dlstp.8 lr, r0
.endr

.irp cond, eq, ne, gt, ge, lt, le
it \cond
letp lr, .label_back
.endr

wlstp.8 lr, pc, .label
wlstp.8 lr, sp, .label
dlstp.16 lr, pc
dlstp.16 lr, sp
.label:
letp .label_back
wlstp.8 lr, r0, .label
letp lr, .label2
.label2:
