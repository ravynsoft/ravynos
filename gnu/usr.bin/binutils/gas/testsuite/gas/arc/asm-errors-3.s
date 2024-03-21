	.cpu ARCHS
.L1:
	bl.d	@foo
	st	1,[@a]
	bl.d	@foo
	st	@a,[r1]
	bl.d	@foo
	breq	r0,r1,@.L1
	bl.d	@foo
	bl 	@foo
	bl.d	@foo
	bbit0	r0,r1,@.L1
	bl.d	@foo
	ei_s	1
