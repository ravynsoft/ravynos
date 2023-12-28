# Power E500MC tests
	.text
start:
	rfdi
	rfgi
	dnh	0, 1023
	dnh	31, 0
	icbiep	9, 10
	msgclr	13
	msgsnd	14
	wait
	wait 0
	waitrsv
	wait 1
	waitimpl
	wait 2
	mdors
	ehpriv
	dsn	24, 25
	lbepx	1, 2, 3
	lhepx	4, 5, 6
	lwepx	7, 8, 9
	ldepx	10, 11, 12
	lfdepx	13, 14, 15
	stbepx	16, 17, 18
	sthepx	19, 20, 21
	stwepx	22, 23, 24
	stdepx	25, 26, 27
	stfdepx	28, 29, 30
	lbdx	0, 1, 2
	lhdx	12, 13, 14
	lwdx	3, 4, 5
	lfddx	26, 27, 28
	lddx	15, 16, 17
	stbdx	6, 7, 8
	sthdx	18, 19, 20
	stwdx	9, 10, 11
	stfddx	29, 30, 31
	stddx	21, 22, 23
	dcbal	0, 1
	dcbzl	6, 7
	dcbstep	31, 0
	dcbfep	1, 2
	dcbtstep 3, 4, 5
	dcbtep	6, 7, 8
	dcbzep	11, 12
	tlbilxlpid
	tlbilxpid
	tlbilxva 2, 3
	tlbilx	3, 4, 5
