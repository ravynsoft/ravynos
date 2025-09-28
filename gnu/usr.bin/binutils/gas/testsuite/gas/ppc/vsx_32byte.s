	.text
_start:
	lxvp	2,0(31)
	lxvp	62,-16(0)
	plxvp	4,1(30)
	plxvp	60,-1(9)
	plxvp	6,0x12345678(0),1
	plxvp	58,_start-.
	lxvpx	56,0,1

	stxvp	8,0(29)
	stxvp	54,-16(0)
	pstxvp	10,1(28)
	pstxvp	52,-1(8)
	pstxvp	12,0x12345678(0),1
	pstxvp	50,_start-.
	stxvpx	48,0,1
