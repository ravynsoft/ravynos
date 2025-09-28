target:
	th.ldia		a0, (a1), -17, 0
	th.ldib		a0, (a1), 0, -1
	th.sdia		a0, (a1), 16, 0
	th.sdib		a0, (a1), 0, 4

	th.ldia		a0, (a0), 0, 0
	th.ldib		a0, (a0), 0, 0

	th.lrd		a0, a1, a2, -1
	th.srd		a0, a1, a2, 4

	th.lurd		a0, a1, a2, -1
	th.surd		a0, a1, a2, 4
