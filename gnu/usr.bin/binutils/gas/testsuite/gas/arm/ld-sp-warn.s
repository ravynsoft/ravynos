.syntax unified
.thumb
ldr sp, [r0, #16]!
ldr sp, [r1], #8
ldr sp, [r0, #16]
ldr r1, [r0, #16]
ldr r1, [r0, r1]!
ldrsb sp, [r2, #16]!
