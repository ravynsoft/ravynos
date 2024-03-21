
.syntax unified
.thumb

vrmlaldavh.s16 r0, r1, q2, q3
vrmlaldavh.i32 r0, r1, q2, q3
vrmlaldavha.s16 r0, r1, q2, q3
vrmlaldavha.i32 r0, r1, q2, q3
vrmlalvh.s16 r0, r1, q2, q3
vrmlalvh.i32 r0, r1, q2, q3
vrmlalvha.s16 r0, r1, q2, q3
vrmlalvha.i32 r0, r1, q2, q3
vrmlaldavhx.u32 r0, r1, q2, q3
vrmlaldavhax.u32 r0, r1, q2, q3
vrmlaldavhx.i32 r0, r1, q2, q3
vrmlaldavhax.i32 r0, r1, q2, q3

vrmlsldavh.s16 r0, r1, q2, q3
vrmlsldavh.u32 r0, r1, q2, q3
vrmlsldavha.s16 r0, r1, q2, q3
vrmlsldavha.u32 r0, r1, q2, q3
vrmlsldavhx.s16 r0, r1, q2, q3
vrmlsldavhx.u32 r0, r1, q2, q3
vrmlsldavhax.s16 r0, r1, q2, q3
vrmlsldavhax.u32 r0, r1, q2, q3

vrmlaldavh.s32 r1, r1, q2, q3
vrmlaldavh.s32 r0, r0, q2, q3
vrmlaldavh.s32 r0, sp, q2, q3
vrmlaldavh.s32 r0, pc, q2, q3
vrmlaldavha.s32 r1, r1, q2, q3
vrmlaldavha.s32 r0, r0, q2, q3
vrmlaldavha.s32 r0, sp, q2, q3
vrmlaldavha.s32 r0, pc, q2, q3
vrmlaldavhx.s32 r1, r1, q2, q3
vrmlaldavhx.s32 r0, r0, q2, q3
vrmlaldavhx.s32 r0, sp, q2, q3
vrmlaldavhx.s32 r0, pc, q2, q3
vrmlaldavhax.s32 r1, r1, q2, q3
vrmlaldavhax.s32 r0, r0, q2, q3
vrmlaldavhax.s32 r0, sp, q2, q3
vrmlaldavhax.s32 r0, pc, q2, q3
vrmlalvh.s32 r1, r1, q2, q3
vrmlalvh.s32 r0, r0, q2, q3
vrmlalvh.s32 r0, sp, q2, q3
vrmlalvh.s32 r0, pc, q2, q3
vrmlalvha.s32 r1, r1, q2, q3
vrmlalvha.s32 r0, r0, q2, q3
vrmlalvha.s32 r0, sp, q2, q3
vrmlalvha.s32 r0, pc, q2, q3

vrmlsldavh.s32 r1, r1, q2, q3
vrmlsldavh.s32 r0, r0, q2, q3
vrmlsldavh.s32 r0, sp, q2, q3
vrmlsldavh.s32 r0, pc, q2, q3
vrmlsldavha.s32 r1, r1, q2, q3
vrmlsldavha.s32 r0, r0, q2, q3
vrmlsldavha.s32 r0, sp, q2, q3
vrmlsldavha.s32 r0, pc, q2, q3
vrmlsldavhx.s32 r1, r1, q2, q3
vrmlsldavhx.s32 r0, r0, q2, q3
vrmlsldavhx.s32 r0, sp, q2, q3
vrmlsldavhx.s32 r0, pc, q2, q3
vrmlsldavhax.s32 r1, r1, q2, q3
vrmlsldavhax.s32 r0, r0, q2, q3
vrmlsldavhax.s32 r0, sp, q2, q3
vrmlsldavhax.s32 r0, pc, q2, q3

.irp op, vrmlaldavh, vrmlaldavha, vrmlaldavhx, vrmlaldavhax, vrmlalvh, vrmlalvha, vrmlsldavh, vrmlsldavha, vrmlsldavhx, vrmlsldavhax

.irp cond, eq, ne, gt, ge, lt, le

it \cond
\op\().s32 r0, r1, q2, q3

.endr

.endr

it eq
vrmlaldavheq.s32 r0, r1, q2, q3
vrmlaldavheq.s32 r0, r1, q2, q3
vpst
vrmlaldavheq.s32 r0, r1, q2, q3
vrmlaldavht.s32 r0, r1, q2, q3
vpst
vrmlaldavh.s32 r0, r1, q2, q3
it eq
vrmlaldavhaeq.s32 r0, r1, q2, q3
vrmlaldavhaeq.s32 r0, r1, q2, q3
vpst
vrmlaldavhaeq.s32 r0, r1, q2, q3
vrmlaldavhat.s32 r0, r1, q2, q3
vpst
vrmlaldavha.s32 r0, r1, q2, q3
it eq
vrmlaldavhxeq.s32 r0, r1, q2, q3
vrmlaldavhxeq.s32 r0, r1, q2, q3
vpst
vrmlaldavhxeq.s32 r0, r1, q2, q3
vrmlaldavhxt.s32 r0, r1, q2, q3
vpst
vrmlaldavhx.s32 r0, r1, q2, q3
it eq
vrmlaldavhaxeq.s32 r0, r1, q2, q3
vrmlaldavhaxeq.s32 r0, r1, q2, q3
vpst
vrmlaldavhaxeq.s32 r0, r1, q2, q3
vrmlaldavhaxt.s32 r0, r1, q2, q3
vpst
vrmlaldavhax.s32 r0, r1, q2, q3
it eq
vrmlalvheq.s32 r0, r1, q2, q3
vrmlalvheq.s32 r0, r1, q2, q3
vpst
vrmlalvheq.s32 r0, r1, q2, q3
vrmlalvht.s32 r0, r1, q2, q3
vpst
vrmlalvh.s32 r0, r1, q2, q3
it eq
vrmlalvhaeq.s32 r0, r1, q2, q3
vrmlalvhaeq.s32 r0, r1, q2, q3
vpst
vrmlalvhaeq.s32 r0, r1, q2, q3
vrmlalvhat.s32 r0, r1, q2, q3
vpst
vrmlalvha.s32 r0, r1, q2, q3
it eq
vrmlsldavheq.s32 r0, r1, q2, q3
vrmlsldavheq.s32 r0, r1, q2, q3
vpst
vrmlsldavheq.s32 r0, r1, q2, q3
vrmlsldavht.s32 r0, r1, q2, q3
vpst
vrmlsldavh.s32 r0, r1, q2, q3
it eq
vrmlsldavhaeq.s32 r0, r1, q2, q3
vrmlsldavhaeq.s32 r0, r1, q2, q3
vpst
vrmlsldavhaeq.s32 r0, r1, q2, q3
vrmlsldavhat.s32 r0, r1, q2, q3
vpst
vrmlsldavha.s32 r0, r1, q2, q3
it eq
vrmlsldavhxeq.s32 r0, r1, q2, q3
vrmlsldavhxeq.s32 r0, r1, q2, q3
vpst
vrmlsldavhxeq.s32 r0, r1, q2, q3
vrmlsldavhxt.s32 r0, r1, q2, q3
vpst
vrmlsldavhx.s32 r0, r1, q2, q3
it eq
vrmlsldavhaxeq.s32 r0, r1, q2, q3
vrmlsldavhaxeq.s32 r0, r1, q2, q3
vpst
vrmlsldavhaxeq.s32 r0, r1, q2, q3
vrmlsldavhaxt.s32 r0, r1, q2, q3
vpst
vrmlsldavhax.s32 r0, r1, q2, q3
