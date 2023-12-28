        .syntax unified
        .text

foo:
	cset	r4, ne
	csetm	r4, ne
	cinc	r3, r2, lt
	cinv	r3, r2, lt
	cneg	r3, r2, lt
	csinc	r3, r2, r4, lt
	csinc	r3, r4, r4, lt
	csinc	r3, zr, zr, lt
	csinv	r3, r2, r4, lt
	csinv	r3, r4, r4, lt
	csinv	r3, zr, zr, lt
	csneg	r3, r2, r4, lt
	csneg	r3, r4, r4, lt
