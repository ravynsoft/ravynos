.syntax unified
.thumb

.macro all_vstld2 op
.irp part, 0, 1
.irp size, .8, .16, .32
.irp op2, r0, r1, r2, r4, r7, r8, r10, r12, r13, r14
\op\()\part\()\size {q0, q1}, [\op2]
\op\()\part\()\size {q1, q2}, [\op2]
\op\()\part\()\size {q2, q3}, [\op2]
\op\()\part\()\size {q3, q4}, [\op2]
\op\()\part\()\size {q4, q5}, [\op2]
\op\()\part\()\size {q5, q6}, [\op2]
\op\()\part\()\size {q6, q7}, [\op2]
.endr
.irp op2, r0, r1, r2, r4, r7, r8, r10, r12,  r14
\op\()\part\()\size {q0, q1}, [\op2]!
\op\()\part\()\size {q1, q2}, [\op2]!
\op\()\part\()\size {q2, q3}, [\op2]!
\op\()\part\()\size {q3, q4}, [\op2]!
\op\()\part\()\size {q4, q5}, [\op2]!
\op\()\part\()\size {q5, q6}, [\op2]!
\op\()\part\()\size {q6, q7}, [\op2]!
.endr
.endr
.endr
.endm

.macro all_vstld4 op
.irp part, 0, 1, 2, 3
.irp size, .8, .16, .32
.irp op2, r0, r1, r2, r4, r7, r8, r10, r12, r13, r14
\op\()\part\()\size {q0, q1, q2, q3}, [\op2]
\op\()\part\()\size {q1, q2, q3, q4}, [\op2]
\op\()\part\()\size {q2, q3, q4, q5}, [\op2]
\op\()\part\()\size {q3, q4, q5, q6}, [\op2]
\op\()\part\()\size {q4, q5, q6, q7}, [\op2]
.endr
.irp op2, r0, r1, r2, r4, r7, r8, r10, r12,  r14
\op\()\part\()\size {q0, q1, q2, q3}, [\op2]!
\op\()\part\()\size {q1, q2, q3, q4}, [\op2]!
\op\()\part\()\size {q2, q3, q4, q5}, [\op2]!
\op\()\part\()\size {q3, q4, q5, q6}, [\op2]!
\op\()\part\()\size {q4, q5, q6, q7}, [\op2]!
.endr
.endr
.endr
.endm
all_vstld2 vst2
all_vstld2 vld2
all_vstld4 vst4
all_vstld4 vld4
