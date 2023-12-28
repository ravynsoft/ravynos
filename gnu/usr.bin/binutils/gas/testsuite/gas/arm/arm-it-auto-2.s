.syntax unified
2:	subs	r2, r2, #64
					@ IT generated automatically
	stmge	r0!, {r1, r3, ip, lr}	@ 64 bytes at a time.
	stmge	r0!, {r1, r3, ip, lr}
	stmge	r0!, {r1, r3, ip, lr}
	stmge	r0!, {r1, r3, ip, lr}
	bgt	2b			@ This should not generate a new IT block
