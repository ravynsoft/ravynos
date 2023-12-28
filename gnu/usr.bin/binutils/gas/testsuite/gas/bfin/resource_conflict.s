	.text

	r6 = r1.h * r2.l || r6 = [p0 ++];
	r6 = [p0++] || r6 = [i0];
	r0 = [i0++] || r1 = [i0--];
	r6.h = ashift r0.h by r1.l || r6 = [i0 ++ m0];
	r6.h = ashift r0.h by r1.l || r6.h = W [p0];
	r6.l = (a0 = r1.l * r2.l) || r6 = [i0--];

	R0 = (A0 += A1) || R0.L = W [I0++];
	R0 = (A0 += A1) || R0.H = W [I0++];
	R0.h = (A0 += A1) || R0.H = W [I0++];
	R0.l = (A0 += A1) || R0.L = W [I0++];

