target:
	th.ldia		a0, (a1), 0, 0
	th.ldib		a0, (a1), 15, 1
	th.lwia		a0, (a1), 0, 2
	th.lwib		a0, (a1), -16, 3
	th.lwuia	a0, (a1), 0, 0
	th.lwuib	a0, (a1), 15, 1
	th.lhia		a0, (a1), 0, 2
	th.lhib		a0, (a1), -16, 3
	th.lhuia	a0, (a1), 0, 0
	th.lhuib	a0, (a1), 15, 1
	th.lbia		a0, (a1), 0, 2
	th.lbib		a0, (a1), -16, 3
	th.lbuia	a0, (a1), 0, 0
	th.lbuib	a0, (a1), 15, 1

	th.sdia		a0, (a1), -16, 0
	th.sdib		a0, (a1), -1, 1
	th.swia		a0, (a1), 0, 2
	th.swib		a0, (a1), 1, 3
	th.shia		a0, (a1), 4, 0
	th.shib		a0, (a1), 13, 1
	th.sbia		a0, (a1), 14, 2
	th.sbib		a0, (a1), 15, 3

	th.lrd		a0, a1, a2, 0
	th.lrw		a0, a1, a2, 1
	th.lrwu		a0, a1, a2, 2
	th.lrh		a0, a1, a2, 3
	th.lrhu		a0, a1, a2, 0
	th.lrb		a0, a1, a2, 1
	th.lrbu		a0, a1, a2, 2
	th.srd		a0, a1, a2, 3
	th.srw		a0, a1, a2, 0
	th.srh		a0, a1, a2, 1
	th.srb		a0, a1, a2, 2

	th.lurd		a0, a1, a2, 0
	th.lurw		a0, a1, a2, 1
	th.lurwu	a0, a1, a2, 2
	th.lurh		a0, a1, a2, 3
	th.lurhu	a0, a1, a2, 0
	th.lurb		a0, a1, a2, 1
	th.lurbu	a0, a1, a2, 2
	th.surd		a0, a1, a2, 3
	th.surw		a0, a1, a2, 0
	th.surh		a0, a1, a2, 1
	th.surb		a0, a1, a2, 2
