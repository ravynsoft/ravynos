.syntax unified
.thumb
.irp op1, r0, r2, r4, r8, r10, r12, r14
.irp op2, r1, r3, r5, r7, r9, r11
.irp op3, q0, q1, q2, q4, q7
.irp op4, q0, q1, q2, q4, q7
.irp data, s32, u32
vrmlaldavh.\data \op1, \op2, \op3, \op4
vrmlaldavha.\data \op1, \op2, \op3, \op4
vrmlalvh.\data \op1, \op2, \op3, \op4
vrmlalvha.\data \op1, \op2, \op3, \op4
.endr
vrmlaldavhx.s32 \op1, \op2, \op3, \op4
vrmlaldavhax.s32 \op1, \op2, \op3, \op4
.endr
.endr
.endr
.endr

.irp op1, r0, r2, r4, r8, r10, r12, r14
.irp op2, r1, r3, r5, r7, r9, r11
.irp op3, q0, q1, q2, q4, q7
.irp op4, q0, q1, q2, q4, q7
vrmlsldavh.s32 \op1, \op2, \op3, \op4
vrmlsldavha.s32 \op1, \op2, \op3, \op4
vrmlsldavhx.s32 \op1, \op2, \op3, \op4
vrmlsldavhax.s32 \op1, \op2, \op3, \op4
.endr
.endr
.endr
.endr

vpstete
vrmlaldavht.s32 r0, r1, q2, q3
vrmlaldavhe.u32 lr, r11, q7, q7
vrmlaldavhat.s32 lr, r11, q7, q7
vrmlaldavhae.u32 r0, r1, q2, q3
vpstete
vrmlaldavhxt.s32 r0, r1, q2, q3
vrmlaldavhxe.s32 r4, r7, q0, q5
vrmlaldavhaxt.s32 r0, r1, q2, q3
vrmlaldavhaxe.s32 lr, r11, q7, q7
vpstete
vrmlalvht.s32 r0, r1, q2, q3
vrmlalvhe.s32 lr, r11, q7, q7
vrmlalvhat.s32 r0, r1, q2, q3
vrmlalvhae.s32 lr, r11, q7, q7
vpstete
vrmlsldavht.s32 r0, r1, q2, q3
vrmlsldavhe.s32 lr, r11, q7, q7
vrmlsldavhat.s32 r0, r1, q2, q3
vrmlsldavhae.s32 lr, r11, q7, q7
vpstete
vrmlsldavhxt.s32 r0, r1, q2, q3
vrmlsldavhxe.s32 lr, r11, q7, q7
vrmlsldavhaxt.s32 r0, r1, q2, q3
vrmlsldavhaxe.s32 lr, r11, q7, q7
