target:
	th.srri		a0, a1, 0
	th.srri		a0, a1, 1
	th.srri		a0, a1, 62
	th.srri		a0, a1, 63
	th.srriw	a0, a1, 0
	th.srriw	a0, a1, 1
	th.srriw	a0, a1, 30
	th.srriw	a0, a1, 31
	th.ext		a0, a1, 1, 0
	th.ext		a0, a1, 31, 0
	th.ext		a0, a1, 63, 31
	th.ext		a0, a1, 63, 62
	th.extu		a0, a1, 1, 0
	th.extu		a0, a1, 31, 0
	th.extu		a0, a1, 63, 31
	th.extu		a0, a1, 63, 62
	th.ff0		a0, a1
	th.ff1		a0, a1
	th.rev		a0, a1
	th.revw		a0, a1
	th.tstnbz	a0, a1
