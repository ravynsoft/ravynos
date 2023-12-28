        .syntax unified
        .text

foo:
	cset	r4, r2, ne
	csetm	sp, ne
	cinc	r3, pc, lt
	cinv	pc, r2, lt
	cneg	r3, sp, lt
	it eq
	csinc	r3, r2, r4, lt
	csinv	r3, r4, r4, lt
	it ne
	csnegne	r3, r2, r4, lt
	csinv	r3, r4, r4, lt
