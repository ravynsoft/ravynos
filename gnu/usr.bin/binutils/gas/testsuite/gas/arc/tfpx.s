	lr         r4,[d1h]
	lr         r3,[d1l]
	lr         r4,[d2h]
	lr         r3,[d2l]

	daddh11	r1,r2,r3
	daddh12	r1,r2,r3
	daddh21	r1,r2,r3
	daddh22	r1,r2,r3

	dexcl1	r1,r2,r3
	dexcl2	r1,r2,r3

	dmulh11	r1,r2,r3
	dmulh12	r1,r2,r3
	dmulh21	r1,r2,r3
	dmulh22	r1,r2,r3

	dsubh11	r1,r2,r3
	dsubh12	r1,r2,r3
	dsubh21	r1,r2,r3
	dsubh22	r1,r2,r3
