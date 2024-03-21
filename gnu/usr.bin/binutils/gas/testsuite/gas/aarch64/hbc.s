	.arch	armv8.8-a

1:
	bc.eq	1b
	bc.ne	1b
	bc.ls	1b
	bc.lo	1b
	bc.hi	1b
	bc.hs	1b
	bc.hi	1b
	bc.lt	1b
	bc.le	1b
	bc.gt	1b
	bc.ge	1b
	bc.cc	1b
	bc.cs	1b
	bc.vs	1b
	bc.vc	1b
	bc.mi	1b
	bc.pl	1b

	bc.ul	1b

	bc.any   1b
	bc.none  1b
	bc.first 1b
	bc.nfrst 1b
	bc.last  1b
	bc.nlast 1b
	bc.pmore 1b
	bc.plast 1b
	bc.tcont 1b
	bc.tstop 1b

	.arch	armv8.7-a+hbc

	bc.eq	1b
