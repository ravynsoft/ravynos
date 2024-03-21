target:
	th.srri		a0, a1, -1
	th.srri		a0, a1, 64
	th.srriw	a0, a1, -1
	th.srriw	a0, a1, 32
	th.ext		a0, 64, 62
	th.extu		a0, -1, 0
