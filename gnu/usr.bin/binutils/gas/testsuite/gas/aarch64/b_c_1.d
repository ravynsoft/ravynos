# objdump: -d

.*: .*


Disassembly of section \.text:

0+0 <\.text>:
.*:	54.....0 	b\.eq	0 <\.text>  // b\.none
.*:	54.....0 	b\.eq	0 <\.text>  // b\.none
.*:	54.....2 	b\.cs	0 <\.text>  // b\.hs, b\.nlast
.*:	54.....2 	b\.cs	0 <\.text>  // b\.hs, b\.nlast
.*:	54.....2 	b\.cs	0 <\.text>  // b\.hs, b\.nlast
.*:	54.....3 	b\.cc	0 <\.text>  // b\.lo, b\.ul, b\.last
.*:	54.....3 	b\.cc	0 <\.text>  // b\.lo, b\.ul, b\.last
.*:	54.....3 	b\.cc	0 <\.text>  // b\.lo, b\.ul, b\.last
.*:	54.....3 	b\.cc	0 <\.text>  // b\.lo, b\.ul, b\.last
.*:	54.....4 	b\.mi	0 <\.text>  // b\.first
.*:	54.....4 	b\.mi	0 <\.text>  // b\.first
.*:	54.....5 	b\.pl	0 <\.text>  // b\.nfrst
.*:	54.....5 	b\.pl	0 <\.text>  // b\.nfrst
.*:	54.....6 	b\.vs	0 <\.text>
.*:	54.....7 	b\.vc	0 <\.text>
.*:	54.....8 	b\.hi	0 <\.text>  // b\.pmore
.*:	54.....8 	b\.hi	0 <\.text>  // b\.pmore
.*:	54.....9 	b\.ls	0 <\.text>  // b\.plast
.*:	54.....9 	b\.ls	0 <\.text>  // b\.plast
.*:	54.....a 	b\.ge	0 <\.text>  // b\.tcont
.*:	54.....a 	b\.ge	0 <\.text>  // b\.tcont
.*:	54.....b 	b\.lt	0 <\.text>  // b\.tstop
.*:	54.....b 	b\.lt	0 <\.text>  // b\.tstop
.*:	54.....c 	b\.gt	0 <\.text>
.*:	54.....d 	b\.le	0 <\.text>
.*:	9a830041 	csel	x1, x2, x3, eq	// eq = none
.*:	9a830041 	csel	x1, x2, x3, eq	// eq = none
.*:	9a832041 	csel	x1, x2, x3, cs	// cs = hs, nlast
.*:	9a832041 	csel	x1, x2, x3, cs	// cs = hs, nlast
.*:	9a832041 	csel	x1, x2, x3, cs	// cs = hs, nlast
.*:	9a833041 	csel	x1, x2, x3, cc	// cc = lo, ul, last
.*:	9a833041 	csel	x1, x2, x3, cc	// cc = lo, ul, last
.*:	9a833041 	csel	x1, x2, x3, cc	// cc = lo, ul, last
.*:	9a833041 	csel	x1, x2, x3, cc	// cc = lo, ul, last
.*:	9a834041 	csel	x1, x2, x3, mi	// mi = first
.*:	9a834041 	csel	x1, x2, x3, mi	// mi = first
.*:	9a835041 	csel	x1, x2, x3, pl	// pl = nfrst
.*:	9a835041 	csel	x1, x2, x3, pl	// pl = nfrst
.*:	9a836041 	csel	x1, x2, x3, vs
.*:	9a837041 	csel	x1, x2, x3, vc
.*:	9a838041 	csel	x1, x2, x3, hi	// hi = pmore
.*:	9a838041 	csel	x1, x2, x3, hi	// hi = pmore
.*:	9a839041 	csel	x1, x2, x3, ls	// ls = plast
.*:	9a839041 	csel	x1, x2, x3, ls	// ls = plast
.*:	9a83a041 	csel	x1, x2, x3, ge	// ge = tcont
.*:	9a83a041 	csel	x1, x2, x3, ge	// ge = tcont
.*:	9a83b041 	csel	x1, x2, x3, lt	// lt = tstop
.*:	9a83b041 	csel	x1, x2, x3, lt	// lt = tstop
.*:	9a83c041 	csel	x1, x2, x3, gt
.*:	9a83d041 	csel	x1, x2, x3, le
