	.text
	.set	noreorder
foo:
	ctc0	$0, $0
	ctc0	$0, $1
	ctc0	$0, $2
	ctc0	$0, $3
	ctc0	$0, $4
	ctc0	$0, $5
	ctc0	$0, $6
	ctc0	$0, $7
	ctc0	$0, $8
	ctc0	$0, $9
	ctc0	$0, $10
	ctc0	$0, $11
	ctc0	$0, $12
	ctc0	$0, $13
	ctc0	$0, $14
	ctc0	$0, $15
	ctc0	$0, $16
	ctc0	$0, $17
	ctc0	$0, $18
	ctc0	$0, $19
	ctc0	$0, $20
	ctc0	$0, $21
	ctc0	$0, $22
	ctc0	$0, $23
	ctc0	$0, $24
	ctc0	$0, $25
	ctc0	$0, $26
	ctc0	$0, $27
	ctc0	$0, $28
	ctc0	$0, $29
	ctc0	$0, $30
	ctc0	$0, $31

	cfc0	$0, $0
	cfc0	$0, $1
	cfc0	$0, $2
	cfc0	$0, $3
	cfc0	$0, $4
	cfc0	$0, $5
	cfc0	$0, $6
	cfc0	$0, $7
	cfc0	$0, $8
	cfc0	$0, $9
	cfc0	$0, $10
	cfc0	$0, $11
	cfc0	$0, $12
	cfc0	$0, $13
	cfc0	$0, $14
	cfc0	$0, $15
	cfc0	$0, $16
	cfc0	$0, $17
	cfc0	$0, $18
	cfc0	$0, $19
	cfc0	$0, $20
	cfc0	$0, $21
	cfc0	$0, $22
	cfc0	$0, $23
	cfc0	$0, $24
	cfc0	$0, $25
	cfc0	$0, $26
	cfc0	$0, $27
	cfc0	$0, $28
	cfc0	$0, $29
	cfc0	$0, $30
	cfc0	$0, $31

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	4, 0
	.space	16
