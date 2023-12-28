.text
.align 0
	cdp2	p1, 4, cr1, cr2, cr3

	ldc2	5, cr9, [r3]
	ldc2l	1, cr14, [r1, #32]
	ldc2	p0, c8, foo
foo:

	stc2	5, cr0, [r3]
	stc2l	3, cr15, [r0, #8]
	stc2	p1, c7, bar
bar:

	mrc2	2, 3, r5, c1, c2
	mcr2	p7, 1, r5, cr1, cr1

	@ The following patterns test Addressing Mode 5 "Unindexed"

        ldc2    5,   c5, [r2], {2}
        stc2    p6,  c4, [r3], {3}
        @ using '9, 10, 11' below results in an invalid ldc2l/stc2l instruction.
        ldc2l   12,   c1, [r6], {6}
        stc2l   p12, c0, [r7], {7}
