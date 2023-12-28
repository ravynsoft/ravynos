1:
	b.eq	1b
	b.none	1b

	b.cs	1b
	b.hs	1b
	b.nlast	1b

	b.cc	1b
	b.lo	1b
	b.ul	1b
	b.last	1b

	b.mi	1b
	b.first	1b

	b.pl	1b
	b.nfrst	1b

	b.vs	1b

	b.vc	1b

	b.hi	1b
	b.pmore	1b

	b.ls	1b
	b.plast	1b

	b.ge	1b
	b.tcont	1b

	b.lt	1b
	b.tstop	1b

	b.gt	1b

	b.le	1b

	csel	x1, x2, x3, eq
	csel	x1, x2, x3, none

	csel	x1, x2, x3, cs
	csel	x1, x2, x3, hs
	csel	x1, x2, x3, nlast

	csel	x1, x2, x3, cc
	csel	x1, x2, x3, lo
	csel	x1, x2, x3, ul
	csel	x1, x2, x3, last

	csel	x1, x2, x3, mi
	csel	x1, x2, x3, first

	csel	x1, x2, x3, pl
	csel	x1, x2, x3, nfrst

	csel	x1, x2, x3, vs

	csel	x1, x2, x3, vc

	csel	x1, x2, x3, hi
	csel	x1, x2, x3, pmore

	csel	x1, x2, x3, ls
	csel	x1, x2, x3, plast

	csel	x1, x2, x3, ge
	csel	x1, x2, x3, tcont

	csel	x1, x2, x3, lt
	csel	x1, x2, x3, tstop

	csel	x1, x2, x3, gt

	csel	x1, x2, x3, le
