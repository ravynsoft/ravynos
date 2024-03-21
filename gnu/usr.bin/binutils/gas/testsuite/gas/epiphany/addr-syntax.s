; Check that we can do negative displacements
	ldr	r1,[r2,-2047]
; Check postmodified immediate with positive and negative displacements
        ldr     r2,[r3],-8
        strd    r8,[r4],8

; Check that zero displacements work
	ldrd	r10,[r12]
	strd	r10,[r14]
