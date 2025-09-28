	.text
start:
	lmw	20,16(10)
	lswi	10,11,1
	lswi	12,11,32
	lswx	10,11,12
	stmw	20,16(10)
	stswi	10,11,1
	stswi	10,11,32
	stswx	10,11,12
