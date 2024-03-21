        .text

        ;; Atomic Operations: exc
        exc  r0,r0,[xa:r2]
        exc  r1,r1,[sd:r1]
        exc  r2,r2,[xd:r3]
        exc  r3,r3,[r1]

        exc.di.f  r12,r12,[xa:r12]
        exc.di.f  r14,r14,[sd:r13]
        exc.di.f  r13,r13,[xd:r14]
        exc.di.f  r15,r15,[r15]

        exc.f  r12,r12,[xa:r0]
        exc.f  r14,r14,[sd:r1]
        exc.f  r13,r13,[xd:r0]
        exc.f  r15,r15,[r2]

        exc.di  r12,r12,[xa:r12]
        exc.di  r14,r14,[sd:r14]
        exc.di  r13,r13,[xd:r13]
        exc.di  r15,r15,[r15]
