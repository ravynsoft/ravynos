	.text
	.set noreorder

movz_insns:
	movnz		$2, $3, $4

integer_insns:
	gsmult		$2, $3, $4
	gsmultu		$5, $6, $7
	gsdmult		$8, $9, $10
	gsdmultu	$11, $12, $13
	gsdiv		$14, $15, $16
	gsdivu		$17, $18, $19
	gsddiv		$20, $21, $22
	gsddivu		$23, $24, $25
	gsmod		$26, $27, $28
	gsmodu		$29, $30, $31
	gsdmod		$2, $3, $4
	gsdmodu		$5, $6, $7
