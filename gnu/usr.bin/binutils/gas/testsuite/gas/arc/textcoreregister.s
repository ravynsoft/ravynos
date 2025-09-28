	.extCoreRegister accX, 56, r|w, can_shortcut
	.extCoreRegister accY, 57, r|w, can_shortcut

	add	accX, r0, r2
	add	accY, r1, r3
	add	r2, accX, r3
	add	accX, accY, accX
