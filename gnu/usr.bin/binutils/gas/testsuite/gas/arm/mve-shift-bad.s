        .syntax unified
        .text

foo:
	asrl	r2, r3, #0
	asrl	r2, r3, #33
	asrl	r1, r3, r5
	lsll	r2, r4, #5
	lsll	r2, r15, r5
	lsrl	r2, r13, #5
	sqrshrl	r2, r3, #64, r3
	sqrshrl	r2, r3, #48, r3
	sqrshrl	r2, r3, r3
	sqrshrl	r2, r3, #40,r3
	sqrshr	r2, r2
	uqrshll	r2, r3, #64, r2
	uqrshll	r2, r3, #48, r2
	uqrshll	r2, r3, r2
	uqrshll	r2, r3, #40, r2
	uqshlgt		r2, #32
	urshrlle	r2, r3, r5
