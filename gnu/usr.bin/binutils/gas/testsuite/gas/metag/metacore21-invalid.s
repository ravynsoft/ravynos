# Check illegal instructions
	.text
_start:
	SETL	[D0.0+D1.0],A0.0,A1.0
	SETL	[D0.0+D0.1],D0.2,D1.2
	SETD	[A0.0+A0.1],A0.2
	ASL	D0.0,D1.0,D0.0
	GETD	D0.0,[D0.0--D0.0]
	SWAP	PC,PCX
	SWAP	CT.0,PCX
	SWAP	D0.1,D0.2
