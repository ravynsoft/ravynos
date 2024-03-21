# Check illegal instructions
	.text
_start:
DL	MOV	D1Ar1,#0xff
DP	MUL	D0Re0, D0Ar6, D0Ar4
DN	MUL	D0Re0, D0Re0, [D0AR.0+D0ARI.1++]
DZ	MUL	[D0BW.1], D0Re0, [D0AR.0+D0ARI.1++]
	ADD     D1Ar1,D1Ar1,#0xff
