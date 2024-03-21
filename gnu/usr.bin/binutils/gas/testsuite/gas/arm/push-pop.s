	.text
	.syntax unified
	push {r0}
	push {r1, r2, r3}
	push {r9}
	pop {r9}
	pop {r1, r2, r3}
	pop {r0}
