	.text
	.set	noreorder
foo:
	dmtc2	$0, $0
	dmtc2	$0, $1
	dmtc2	$0, $2
	dmtc2	$0, $3
	dmtc2	$0, $4
	dmtc2	$0, $5
	dmtc2	$0, $6
	dmtc2	$0, $7
	dmtc2	$0, $8
	dmtc2	$0, $9
	dmtc2	$0, $10
	dmtc2	$0, $11
	dmtc2	$0, $12
	dmtc2	$0, $13
	dmtc2	$0, $14
	dmtc2	$0, $15
	dmtc2	$0, $16
	dmtc2	$0, $17
	dmtc2	$0, $18
	dmtc2	$0, $19
	dmtc2	$0, $20
	dmtc2	$0, $21
	dmtc2	$0, $22
	dmtc2	$0, $23
	dmtc2	$0, $24
	dmtc2	$0, $25
	dmtc2	$0, $26
	dmtc2	$0, $27
	dmtc2	$0, $28
	dmtc2	$0, $29
	dmtc2	$0, $30
	dmtc2	$0, $31

	dmfc2	$0, $0
	dmfc2	$0, $1
	dmfc2	$0, $2
	dmfc2	$0, $3
	dmfc2	$0, $4
	dmfc2	$0, $5
	dmfc2	$0, $6
	dmfc2	$0, $7
	dmfc2	$0, $8
	dmfc2	$0, $9
	dmfc2	$0, $10
	dmfc2	$0, $11
	dmfc2	$0, $12
	dmfc2	$0, $13
	dmfc2	$0, $14
	dmfc2	$0, $15
	dmfc2	$0, $16
	dmfc2	$0, $17
	dmfc2	$0, $18
	dmfc2	$0, $19
	dmfc2	$0, $20
	dmfc2	$0, $21
	dmfc2	$0, $22
	dmfc2	$0, $23
	dmfc2	$0, $24
	dmfc2	$0, $25
	dmfc2	$0, $26
	dmfc2	$0, $27
	dmfc2	$0, $28
	dmfc2	$0, $29
	dmfc2	$0, $30
	dmfc2	$0, $31

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	4, 0
	.space	16
