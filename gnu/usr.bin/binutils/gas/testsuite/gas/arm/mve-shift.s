        .syntax unified
        .text
foo:
	asrl	r2, r3, #5
	asrl	r2, r3, r5
	lsll	r2, r3, #5
	lsll	r2, r3, r5
	lsrl	r2, r3, #5
	sqrshrl	r2, r3, #48, r5
	sqrshrl	r2, r3, #64, r5
	sqrshr	r2, r5
	sqshll	r2, r3, #5
	sqshl	r2, #5
	srshrl	r2, r3, #31
	srshr	r2, #31
	uqrshll	r2, r3, #48, r5
	uqrshll	r2, r3, #64, r5
	uqrshl	r2, r5
	uqshll	r2, r3, #31
	itee	gt
	uqshlgt		r2, #32
	urshrlle	r2, r3, #32
	urshrle		r2, #32
