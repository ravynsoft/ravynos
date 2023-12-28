	.text
power7:
	lxvd2x    3,4,5
	lxvd2x    43,4,5
	stxvd2x   3,4,5
	stxvd2x   43,4,5
	xxmrghd   3,4,5
	xxmrghd   43,44,45
	xxmrgld   3,4,5
	xxmrgld   43,44,45
	xxpermdi  3,4,5,0
	xxpermdi  43,44,45,0
	xxpermdi  3,4,5,3
	xxpermdi  43,44,45,3
	xxpermdi  3,4,5,1
	xxpermdi  43,44,45,1
	xxpermdi  3,4,5,2
	xxpermdi  43,44,45,2
	xvmovdp   3,4
	xvmovdp   43,44
	xvcpsgndp 3,4,4
	xvcpsgndp 43,44,44
	xvcpsgndp 3,4,5
	xvcpsgndp 43,44,45
	doze
	nap
	sleep
	rvwinkle
	prtyw     3,4
	prtyd     13,14
	mfcfar    10
	mtcfar    11
	cmpb      3,4,5
	lwzcix    10,11,12
	dadd      16,17,18
	daddq     20,22,24
	dss       3
	dssall  
	dst       5,4,1
	dstt      8,7,0
	dstst     5,6,3
	dststt    4,5,2
	divwe	  10,11,12
	divwe.	  11,12,13
	divweo	  12,13,14
	divweo.	  13,14,15
	divweu	  10,11,12
	divweu.	  11,12,13
	divweuo	  12,13,14
	divweuo.  13,14,15
	bpermd    7,17,27
	popcntw   10,20
	popcntd   10,20
	ldbrx     20,21,22
	stdbrx    20,21,22
	lfiwzx	  10,0,10
	lfiwzx	  10,9,10
	fcfids    4,5
	fcfids.   4,5
	fcfidus   4,5
	fcfidus.  4,5
	fctiwu    4,5
	fctiwu.   4,5
	fctiwuz   4,5
	fctiwuz.  4,5
	fctidu    4,5
	fctidu.   4,5
	fctiduz   4,5
	fctiduz.  4,5
	fcfidu    4,5
	fcfidu.   4,5
	ftdiv     0,10,11
	ftdiv     7,10,11
	ftsqrt    0,10
	ftsqrt    7,10
	dcbtt     8,9
	dcbtstt   8,9
	dcffix    10,12
	dcffix.   20,22
	fre       14,15
	fre.      14,15
	fres      14,15
	fres.     14,15
	frsqrte   14,15
	frsqrte.  14,15
	frsqrtes  14,15
	frsqrtes. 14,15
	isel	  2,3,4,28
	yield
	or	  27,27,27
	ori	  2,2,0
	.p2align 4,,15
	mdoio
	or	  29,29,29
	mdoom
	or	  30,30,30
	tlbie     10,11
